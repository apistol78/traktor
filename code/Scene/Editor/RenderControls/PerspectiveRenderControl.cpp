/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/IWorldComponentEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/RenderControls/PerspectiveRenderControl.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/AspectLayout.h"
#include "Ui/Itf/IWidget.h"
#include "World/Entity.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Editor/IDebugOverlay.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::scene
{
	namespace
	{

const float c_defaultFieldOfView = 80.0f;
const float c_defaultMouseWheelRate = 10.0f;
const int32_t c_defaultMultiSample = 0;
const float c_defaultMovementSpeed = 40.0f;
const float c_minFieldOfView = 4.0f;
const float c_maxFieldOfView = 160.0f;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

Vector4 projectUnit(const ui::Rect& rc, const ui::Point& pnt)
{
	return Vector4(
		2.0f * float(pnt.x - rc.left) / rc.getWidth() - 1.0f,
		1.0f - 2.0f * float(pnt.y - rc.top) / rc.getHeight(),
		0.0f,
		1.0f
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PerspectiveRenderControl", PerspectiveRenderControl, ISceneRenderControl)

PerspectiveRenderControl::PerspectiveRenderControl()
:	m_fieldOfView(c_defaultFieldOfView)
,	m_mouseWheelRate(c_defaultMouseWheelRate)
,	m_multiSample(c_defaultMultiSample)
{
}

bool PerspectiveRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, const TypeInfo& worldRendererType)
{
	m_context = context;
	T_ASSERT(m_context);

	m_worldRendererType = &worldRendererType;

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);
	m_multiSample = settings->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);
	m_model.setMovementSpeed(settings->getProperty< float >(L"SceneEditor.MovementSpeed", c_defaultMovementSpeed));

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect, ui::WsFocus | ui::WsNoCanvas))
	{
		destroy();
		return false;
	}

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_multiSample;
	desc.multiSampleShading = settings->getProperty< float >(L"Editor.MultiSampleShading", 0.0f);
	desc.waitVBlanks = 1;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
	{
		destroy();
		return false;
	}

	m_renderContext = new render::RenderContext(16 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(
		m_context->getRenderSystem(),
		m_multiSample,
		[=, this](int32_t pass, int32_t level, const std::wstring& name, double start, double duration) {
			m_context->raiseMeasurement(pass, level, name, start, duration);
		}
	);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		1
	))
	{
		destroy();
		return false;
	}

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(m_context->getRenderSystem()))
	{
		destroy();
		return false;
	}
	
	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &PerspectiveRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &PerspectiveRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &PerspectiveRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &PerspectiveRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &PerspectiveRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::KeyDownEvent >(this, &PerspectiveRenderControl::eventKeyDown);
	m_renderWidget->addEventHandler< ui::KeyUpEvent >(this, &PerspectiveRenderControl::eventKeyUp);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &PerspectiveRenderControl::eventPaint);

	updateSettings();

	m_worldRenderView.setIndex(cameraId);

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(true);

	m_dirtySize.cx = m_renderView->getWidth();
	m_dirtySize.cy = m_renderView->getHeight();

	m_renderWidget->startTimer(1000 / 60);
	return true;
}

void PerspectiveRenderControl::destroy()
{
	if (m_camera)
	{
		m_camera->setEnable(false);
		m_camera = nullptr;
	}

	m_renderContext = nullptr;

	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_screenRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void PerspectiveRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	if (m_worldRendererType != &worldRendererType)
	{
		m_worldRendererType = &worldRendererType;
		safeDestroy(m_worldRenderer);
	}
}

void PerspectiveRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void PerspectiveRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	m_imageProcessQuality = imageProcess;
	m_shadowQuality = shadows;
	m_reflectionsQuality = reflections;
	m_motionBlurQuality = motionBlur;
	m_ambientOcclusionQuality = ambientOcclusion;
	m_antiAliasQuality = antiAlias;
	safeDestroy(m_worldRenderer);
}

void PerspectiveRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
	m_overlay = overlay;
}

void PerspectiveRenderControl::setDebugOverlayAlpha(float alpha, float mip)
{
	m_overlayAlpha = alpha;
	m_overlayMip = mip;
}

bool PerspectiveRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.PropertiesChanged")
	{
		Ref< scene::Scene > sceneInstance = m_context->getScene();
		if (!sceneInstance)
			return false;

		const uint32_t hash = DeepHash(sceneInstance->getWorldRenderSettings()).get();
		if (m_worldRendererHash == hash)
			return false;

		safeDestroy(m_worldRenderer);
	}
	else if (command == L"Editor.SettingsChanged")
		updateSettings();
	else if (command == L"Scene.Editor.EnableGrid")
		m_gridEnable = true;
	else if (command == L"Scene.Editor.DisableGrid")
		m_gridEnable = false;
	else if (command == L"Scene.Editor.EnableGuide")
		m_guideEnable = true;
	else if (command == L"Scene.Editor.DisableGuide")
		m_guideEnable = false;

	return result;
}

void PerspectiveRenderControl::update()
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.update(this, m_renderWidget, m_context, transformChain);

#if defined(_WIN32)
	m_renderWidget->update(nullptr, true);
#else
	m_renderWidget->update(nullptr, false);
#endif
}

bool PerspectiveRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	const Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	const ui::Rect innerRect = m_renderWidget->getInnerRect();

	const Scalar fx(float(position.x) / innerRect.getWidth());
	const Scalar fy(float(position.y) / innerRect.getHeight());

	// Interpolate frustum edges to find view pick-ray.
	const Vector4& viewEdgeTopLeft = viewFrustum.corners[4];
	const Vector4& viewEdgeTopRight = viewFrustum.corners[5];
	const Vector4& viewEdgeBottomLeft = viewFrustum.corners[7];
	const Vector4& viewEdgeBottomRight = viewFrustum.corners[6];

	const Vector4 viewEdgeTop = lerp(viewEdgeTopLeft, viewEdgeTopRight, fx);
	const Vector4 viewEdgeBottom = lerp(viewEdgeBottomLeft, viewEdgeBottomRight, fx);
	const Vector4 viewRayDirection = lerp(viewEdgeTop, viewEdgeBottom, fy).normalized().xyz0();

	// Transform ray into world space.
	const Matrix44 viewInv = m_worldRenderView.getView().inverse();
	outWorldRayOrigin = viewInv.translation().xyz1();
	outWorldRayDirection = viewInv * viewRayDirection;

	return true;
}

bool PerspectiveRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	Vector4 origin[4], direction[4];
	calculateRay(rc.getTopRight(), origin[0], direction[0]);
	calculateRay(rc.getTopLeft(), origin[1], direction[1]);
	calculateRay(rc.getBottomLeft(), origin[2], direction[2]);
	calculateRay(rc.getBottomRight(), origin[3], direction[3]);

	const Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	const Scalar nz = viewFrustum.getNearZ();
	const Scalar fz = viewFrustum.getFarZ();

	const Vector4 corners[8] =
	{
		origin[0] + direction[0] * nz,
		origin[0] + direction[1] * nz,
		origin[0] + direction[2] * nz,
		origin[0] + direction[3] * nz,
		origin[0] + direction[0] * fz,
		origin[0] + direction[1] * fz,
		origin[0] + direction[2] * fz,
		origin[0] + direction[3] * fz,
	};

	const Plane planes[6] =
	{
		Plane(corners[1], corners[6], corners[5]),
		Plane(corners[3], corners[4], corners[7]),
		Plane(corners[2], corners[7], corners[6]),
		Plane(corners[0], corners[5], corners[4]),
		Plane(corners[0], corners[2], corners[1]),
		Plane(corners[4], corners[6], corners[7])
	};

	outWorldFrustum.buildFromPlanes(planes);
	return true;
}

bool PerspectiveRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void PerspectiveRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	Vector4 delta = mouseDelta;
	switch (mode)
	{
	case McmRotate:
		delta *= Scalar(c_cameraRotateDeltaScale);
		m_camera->rotate(delta.y(), delta.x());
		break;

	case McmMove:
		delta *= Scalar(c_cameraTranslateDeltaScale);
		m_camera->move(delta);
		break;

	case McmMoveXZ:
		delta *= Scalar(c_cameraTranslateDeltaScale);
		m_camera->move(delta.shuffle< 0, 2, 1, 3 >());
		break;

	case McmMoveXY:
		if (!m_invertPanY)
			delta *= Vector4(c_cameraTranslateDeltaScale, -c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		else
			delta *= Vector4(c_cameraTranslateDeltaScale, c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		m_camera->move(delta.shuffle< 0, 1, 2, 3 >());
		break;
	}
}

void PerspectiveRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void PerspectiveRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		editorProfile->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, *m_worldRendererType, entityRenderers);
		for (auto entityRenderer : entityRenderers)
			worldEntityRenderers->add(entityRenderer);
	}

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(m_worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &m_worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.quality.motionBlur = m_motionBlurQuality;
	wcd.quality.shadows = m_shadowQuality;
	wcd.quality.reflections = m_reflectionsQuality;
	wcd.quality.ambientOcclusion = m_ambientOcclusionQuality;
	wcd.quality.antiAlias = m_antiAliasQuality;
	wcd.quality.imageProcess = m_imageProcessQuality;
	wcd.multiSample = m_multiSample;

	if (!worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		wcd
	))
		return;

	m_worldRenderer = worldRenderer;
	m_worldRendererHash = DeepHash(sceneInstance->getWorldRenderSettings()).get();
}

void PerspectiveRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_colorGrid = settings->getProperty< Color4ub >(L"Editor.Colors/Grid");
	m_colorRef = settings->getProperty< Color4ub >(L"Editor.Colors/ReferenceEdge");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);
	m_model.setMovementSpeed(settings->getProperty< float >(L"SceneEditor.MovementSpeed", c_defaultMovementSpeed));
}

Matrix44 PerspectiveRenderControl::getProjectionTransform() const
{
	return m_worldRenderView.getProjection();
}

Matrix44 PerspectiveRenderControl::getViewTransform() const
{
	return m_camera->getView().toMatrix44();
}

void PerspectiveRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	int32_t rotation = event->getRotation();
	if (!m_model.isMovingCamera())
	{
		if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
			rotation = -rotation;

		m_camera->move(Vector4(0.0f, 0.0f, rotation * -m_mouseWheelRate, 0.0f));
		m_context->raiseCameraMoved();
		m_context->enqueueRedraw(this);
	}
	else
	{
		float movementSpeed = m_model.getMovementSpeed();
		movementSpeed = std::max(movementSpeed + rotation * 20.0f, 1.0f);
		m_model.setMovementSpeed(movementSpeed);
	}
}

void PerspectiveRenderControl::eventKeyDown(ui::KeyDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventKeyDown(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventKeyUp(ui::KeyUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventKeyUp(this, m_renderWidget, event, m_context, transformChain);
}

void PerspectiveRenderControl::eventPaint(ui::PaintEvent* event)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance || !m_renderView || !m_primitiveRenderer)
		return;

	// Render view events; reset view if it has become lost.
	bool lost = false;
	for (render::RenderEvent re = {}; m_renderView->nextEvent(re); )
	{
		if (re.type == render::RenderEventType::Lost)
			lost = true;
	}

	// Check if size has changed since last render; need to reset renderer if so.
	const ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (lost || sz.cx != m_dirtySize.cx || sz.cy != m_dirtySize.cy)
	{
		if (!m_renderView->reset(sz.cx, sz.cy))
			return;
		m_dirtySize = sz;
	}

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	float colorClear[4]; m_colorClear.getRGBA32F(colorClear);
	const double deltaTime = m_timer.getDeltaTime();
	const double scaledTime = m_context->getTime();
	const Matrix44 projection = getProjectionTransform();
	const Matrix44 view = getViewTransform();

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(m_fieldOfView),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);
	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, 0, nullptr);

	// Draw debug overlay, content of any target from render graph as an overlay.
	if (m_overlay)
	{
		m_overlay->setup(
			*m_renderGraph,
			m_screenRenderer,
			m_worldRenderer,
			m_worldRenderView,
			m_overlayAlpha,
			m_overlayMip
		);
	}

	// Draw debug wires.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
		rp->setOutput(0, render::TfAll, render::TfAll);
		rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {

			// Render wire guides.
			m_primitiveRenderer->begin(0, projection);
			m_primitiveRenderer->setClipDistance(m_worldRenderView.getViewFrustum().getNearZ());
			m_primitiveRenderer->pushView(view);

			// Render XZ grid.
			if (m_gridEnable)
			{
				const Vector4 viewPosition = view.inverse().translation();
				const float vx = floorf(viewPosition.x());
				const float vz = floorf(viewPosition.z());

				for (int32_t x = -20; x <= 20; ++x)
				{
					const float fx = float(x);
					m_primitiveRenderer->drawLine(
						Vector4(fx + vx, 0.0f, -20.0f + vz, 1.0f),
						Vector4(fx + vx, 0.0f, 20.0f + vz, 1.0f),
						(int(fx + vx) == 0) ? 2.0f : 0.0f,
						m_colorGrid
					);
					m_primitiveRenderer->drawLine(
						Vector4(-20.0f + vx, 0.0f, fx + vz, 1.0f),
						Vector4(20.0f + vx, 0.0f, fx + vz, 1.0f),
						(int(fx + vz) == 0) ? 2.0f : 0.0f,
						m_colorGrid
					);
				}

				// Draw frame.
				const float c_arrowLength = 0.4f;
				const float c_frameSize = 0.12f;
				const float c_margin = 0.01f;

				const float w = 2.0f * float(sz.cx) / sz.cy;
				const float h = 2.0f;

				m_primitiveRenderer->setProjection(orthoLh(-w / 2.0f, -h / 2.0f, w / 2.0f, h / 2.0f, -1.0f, 1.0f));
				m_primitiveRenderer->pushWorld(Matrix44::identity());
				m_primitiveRenderer->pushView(
					translate(w / 2.0f - (c_frameSize + c_margin), h / 2.0f - (c_frameSize + c_margin), 0.0f) *
					scale(c_frameSize, c_frameSize, c_frameSize)
				);

				m_primitiveRenderer->pushDepthState(false, true, false);
				m_primitiveRenderer->drawSolidQuad(
					Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
					Vector4(1.0f, 1.0f, 1.0f, 1.0f),
					Vector4(1.0f, -1.0f, 1.0f, 1.0f),
					Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 32)
				);
				m_primitiveRenderer->popDepthState();

				m_primitiveRenderer->pushDepthState(true, true, false);

				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + view.axisX() * Scalar(1.0f - c_arrowLength),
					Color4ub(255, 0, 0, 255)
				);
				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + view.axisY() * Scalar(1.0f - c_arrowLength),
					Color4ub(0, 255, 0, 255)
				);
				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + view.axisZ() * Scalar(1.0f - c_arrowLength),
					Color4ub(0, 0, 255, 255)
				);

				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + view.axisX() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + view.axisX(),
					0.8f,
					Color4ub(255, 0, 0, 255)
				);
				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + view.axisY() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + view.axisY(),
					0.8f,
					Color4ub(0, 255, 0, 255)
				);
				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + view.axisZ() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + view.axisZ(),
					0.8f,
					Color4ub(0, 0, 255, 255)
				);

				m_primitiveRenderer->popWorld();
				m_primitiveRenderer->popView();
				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->setProjection(projection);
			}

			// Draw guides.
			if (m_guideEnable)
			{
				for (auto entityAdapter : m_context->getEntities(SceneEditorContext::GfDefault))
					entityAdapter->drawGuides(m_primitiveRenderer, sz);

				// Draw controller guides.
				IWorldComponentEditor* controllerEditor = m_context->getControllerEditor();
				if (controllerEditor)
					controllerEditor->draw(m_primitiveRenderer);
			}

			// Draw modifier.
			IModifier* modifier = m_context->getModifier();
			if (modifier)
				modifier->draw(m_primitiveRenderer);

			// Draw selection rectangle if non-empty.
			if (m_selectionRectangle.area() > 0)
			{
				const ui::Rect innerRect = m_renderWidget->getInnerRect();

				m_primitiveRenderer->setProjection(orthoLh(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f));

				m_primitiveRenderer->pushView(Matrix44::identity());
				m_primitiveRenderer->pushDepthState(false, false, false);

				m_primitiveRenderer->drawSolidQuad(
					projectUnit(innerRect, m_selectionRectangle.getTopLeft()),
					projectUnit(innerRect, m_selectionRectangle.getTopRight()),
					projectUnit(innerRect, m_selectionRectangle.getBottomRight()),
					projectUnit(innerRect, m_selectionRectangle.getBottomLeft()),
					Color4ub(0, 64, 128, 128)
				);
				m_primitiveRenderer->drawWireQuad(
					projectUnit(innerRect, m_selectionRectangle.getTopLeft()),
					projectUnit(innerRect, m_selectionRectangle.getTopRight()),
					projectUnit(innerRect, m_selectionRectangle.getBottomRight()),
					projectUnit(innerRect, m_selectionRectangle.getBottomLeft()),
					Color4ub(120, 190, 250, 255)
				);

				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->popView();
			}

			m_primitiveRenderer->end(0);

			auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
			rb->lambda = [&](render::IRenderView* renderView) {
				m_primitiveRenderer->render(m_renderView, 0);
			};
			renderContext->draw(rb);
		});
		m_renderGraph->addPass(rp);
	}

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, m_dirtySize.cx, m_dirtySize.cy);

	// Render frame.
	if (m_renderView->beginFrame())
	{
		m_renderContext->render(m_renderView);
		m_renderView->endFrame();
		m_renderView->present();
	}

	event->consume();
}

}
