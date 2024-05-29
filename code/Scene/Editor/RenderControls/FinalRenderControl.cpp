/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/RenderControls/FinalRenderControl.h"
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

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FinalRenderControl", FinalRenderControl, ISceneRenderControl)

FinalRenderControl::FinalRenderControl()
:	m_imageProcessQuality(world::Quality::Disabled)
,	m_shadowQuality(world::Quality::Disabled)
,	m_reflectionsQuality(world::Quality::Disabled)
,	m_motionBlurQuality(world::Quality::Disabled)
,	m_ambientOcclusionQuality(world::Quality::Disabled)
,	m_antiAliasQuality(world::Quality::Disabled)
,	m_fieldOfView(c_defaultFieldOfView)
,	m_mouseWheelRate(c_defaultMouseWheelRate)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool FinalRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, const TypeInfo& worldRendererType)
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
	if (!m_renderWidget->create(m_containerAspect, ui::WsFocus | ui::WsWantAllInput | ui::WsNoCanvas))
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

	if (m_context->getDocument()->getInstance(0)->getPrimaryType() == &type_of< SceneAsset >())
	{
		resource::Id< scene::Scene > sceneId(m_context->getDocument()->getInstance(0)->getGuid());
		if (!m_context->getResourceManager()->bind(
			sceneId,
			m_sceneInstance
		))
		{
			destroy();
			return false;
		}
	}

	m_renderContext = new render::RenderContext(16 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_context->getRenderSystem(), m_multiSample);

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(m_context->getRenderSystem()))
	{
		destroy();
		return false;
	}

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &FinalRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &FinalRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &FinalRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &FinalRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &FinalRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::KeyDownEvent >(this, &FinalRenderControl::eventKeyDown);
	m_renderWidget->addEventHandler< ui::KeyUpEvent >(this, &FinalRenderControl::eventKeyUp);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &FinalRenderControl::eventPaint);

	updateSettings();

	m_worldRenderView.setIndex(cameraId);

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(true);

	return true;
}

void FinalRenderControl::destroy()
{
	if (m_camera)
	{
		m_camera->setEnable(false);
		m_camera = nullptr;
	}

	if (m_sceneInstance)
		m_sceneInstance->destroy();

	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_screenRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void FinalRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	m_worldRendererType = &worldRendererType;
	safeDestroy(m_worldRenderer);
}

void FinalRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void FinalRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	m_imageProcessQuality = imageProcess;
	m_shadowQuality = shadows;
	m_reflectionsQuality = reflections;
	m_motionBlurQuality = motionBlur;
	m_ambientOcclusionQuality = ambientOcclusion;
	m_antiAliasQuality = antiAlias;
	safeDestroy(m_worldRenderer);
}

void FinalRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
	m_overlay = overlay;
}

void FinalRenderControl::setDebugOverlayAlpha(float alpha, float mip)
{
	m_overlayAlpha = alpha;
	m_overlayMip = mip;
}

bool FinalRenderControl::handleCommand(const ui::Command& command)
{
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
	return false;
}

void FinalRenderControl::update()
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

bool FinalRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool FinalRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

bool FinalRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void FinalRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
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

void FinalRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
}

void FinalRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	if (!m_sceneInstance)
		return;

	m_worldRenderSettings = *(m_sceneInstance->getWorldRenderSettings());

	// Use world render settings from non-baked scene, still need to
	// keep irradiance grid in order to preview baked irradiance in editor.
	if (m_context->getScene() != nullptr)
	{
		m_worldRenderSettings = *(m_context->getScene()->getWorldRenderSettings());
		m_worldRenderSettings.irradianceGrid = m_sceneInstance->getWorldRenderSettings()->irradianceGrid;
	}

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		editorProfile->createEntityRenderers(m_context, m_renderView, nullptr, *m_worldRendererType, entityRenderers);
		for (auto entityRenderer : entityRenderers)
			worldEntityRenderers->add(entityRenderer);
	}

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

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
	m_worldRendererHash = DeepHash(m_sceneInstance->getWorldRenderSettings()).get();
}

void FinalRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);
	m_model.setMovementSpeed(settings->getProperty< float >(L"SceneEditor.MovementSpeed", c_defaultMovementSpeed));
}

Matrix44 FinalRenderControl::getProjectionTransform() const
{
	return m_worldRenderView.getProjection();
}

Matrix44 FinalRenderControl::getViewTransform() const
{
	return m_camera->getView().toMatrix44();
}

void FinalRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
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

void FinalRenderControl::eventKeyDown(ui::KeyDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventKeyDown(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventKeyUp(ui::KeyUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventKeyUp(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventPaint(ui::PaintEvent* event)
{
	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		m_worldRenderer = nullptr;
		m_sceneInstance.consume();
	}

	if (!m_sceneInstance || !m_renderView)
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
	const double scaledTime = m_context->getTime();
	const float deltaTime = (float)m_timer.getDeltaTime();
	const float scaledDeltaTime = m_context->isPlaying() ? deltaTime * m_context->getTimeScale() : 0.0f;
	const Matrix44 projection = getProjectionTransform();
	const Matrix44 view = getViewTransform();

	// Update scene entities; final render control has it's own set of entities thus
	// need to manually update those.
	world::UpdateParams update;
	update.totalTime = scaledTime;
	update.alternateTime = scaledTime;
	update.deltaTime = scaledDeltaTime;
	m_sceneInstance->update(update);

	//// Build a root entity by gathering entities from containers.
	//Ref< world::GroupComponent > rootGroup = new world::GroupComponent();
	//Ref< world::Entity > rootEntity = new world::Entity();
	//rootEntity->setComponent(rootGroup);

	//m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootGroup->addEntity(entity); });
	//rootGroup->addEntity(m_sceneInstance->getRootEntity());

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
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
	m_worldRenderer->setup(m_sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, 0, nullptr);

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

	// Need to clear all entities from our root group since when our root entity
	// goes out of scope it's automatically destroyed.
	//rootGroup->removeAllEntities();

	event->consume();
}

}
