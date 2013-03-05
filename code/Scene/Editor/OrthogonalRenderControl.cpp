#include <limits>
#include "Core/Math/Log2.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Itf/IWidget.h"
#include "World/IEntityEventManager.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Forward/WorldRendererForward.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_defaultMultiSample = 0;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_minMagnification = 0.01f;

int32_t translateMouseButton(int32_t uimb)
{
	if (uimb == ui::MouseEvent::BtLeft)
		return 0;
	else if (uimb == ui::MouseEvent::BtRight)
		return 1;
	else if (uimb == ui::MouseEvent::BtMiddle)
		return 2;
	else if (uimb == (ui::MouseEvent::BtLeft | ui::MouseEvent::BtRight))
		return 2;
	else
		return 0;
}

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OrthogonalRenderControl", OrthogonalRenderControl, ISceneRenderControl)

OrthogonalRenderControl::OrthogonalRenderControl()
:	m_shadowQuality(world::QuDisabled)
,	m_ambientOcclusionQuality(world::QuDisabled)
,	m_antiAliasQuality(world::QuDisabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_multiSample(0)
,	m_viewPlane(PositiveX)
,	m_viewFarZ(0.0f)
,	m_magnification(10.0f)
,	m_dirtySize(0, 0)
{
}

bool OrthogonalRenderControl::create(ui::Widget* parent, SceneEditorContext* context, ViewPlane viewPlane, int32_t cameraId)
{
	m_context = context;
	T_ASSERT (m_context);

	m_multiSample = m_context->getEditor()->getSettings()->getProperty< PropertyInteger >(L"Editor.MultiSample", c_defaultMultiSample);
	m_viewPlane = viewPlane;

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_multiSample;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonUp));
	m_renderWidget->addDoubleClickEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventDoubleClick));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventPaint));

	updateSettings();
	updateWorldRenderer();

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(false);

	switch (m_viewPlane)
	{
	case PositiveX:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(90.0f))));
		break;
	case NegativeX:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(-90.0f))));
		break;
	case PositiveY:
		m_camera->setOrientation(Quaternion(rotateX(deg2rad(90.0f))));
		break;
	case NegativeY:
		m_camera->setOrientation(Quaternion(rotateX(deg2rad(-90.0f))));
		break;
	case PositiveZ:
		m_camera->setOrientation(Quaternion::identity());
		break;
	case NegativeZ:
		m_camera->setOrientation(Quaternion(rotateY(deg2rad(180.0f))));
		break;
	}

	m_timer.start();
	return true;
}

void OrthogonalRenderControl::destroy()
{
	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	if (m_primitiveRenderer)
	{
		m_primitiveRenderer->destroy();
		m_primitiveRenderer = 0;
	}

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	if (m_renderWidget)
	{
		m_renderWidget->destroy();
		m_renderWidget = 0;
	}
}

void OrthogonalRenderControl::updateWorldRenderer()
{
	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< const world::WorldRenderSettings > worldRenderSettings = sceneInstance->getWorldRenderSettings();

	// Create entity renderers; every renderer is wrapped in a custom renderer in order to check flags etc.
	Ref< EntityRendererCache > entityRendererCache = new EntityRendererCache(m_context);
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
		{
			Ref< EntityRendererAdapter > entityRenderer = new EntityRendererAdapter(entityRendererCache, *j);
			worldEntityRenderers->add(entityRenderer);
		}
	}

	// Create world renderer.
	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.multiSample = m_multiSample;
	wcd.frameCount = 1;

	Ref< world::IWorldRenderer > worldRenderer = new world::WorldRendererForward();
	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		m_renderView,
		wcd
	))
		m_worldRenderer = worldRenderer;

	m_viewFarZ = worldRenderSettings->viewFarZ;
}

void OrthogonalRenderControl::setAspect(float aspect)
{
}

void OrthogonalRenderControl::setQuality(world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	m_shadowQuality = shadowQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	updateWorldRenderer();
}

bool OrthogonalRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.SettingsChanged")
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

void OrthogonalRenderControl::update()
{
	m_renderWidget->update();
}

bool OrthogonalRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

bool OrthogonalRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	const float c_viewFarOffset = 1.0f;
	const ui::Rect innerRect = m_renderWidget->getInnerRect();

	Matrix44 projection = getProjectionTransform();
	Matrix44 projectionInverse = projection.inverse();

	Matrix44 view = getViewTransform();
	Matrix44 viewInverse = view.inverse();

	Scalar fx( float(position.x * 2.0f) / innerRect.getWidth() - 1.0f);
	Scalar fy(-float(position.y * 2.0f) / innerRect.getHeight() + 1.0f);

	Vector4 clipPosition(fx, fy, 0.0f, 1.0f);
	Vector4 viewPosition = projectionInverse * clipPosition + Vector4(0.0f, 0.0f, -(m_viewFarZ - c_viewFarOffset));
	outWorldRayOrigin = viewInverse * viewPosition;
	outWorldRayDirection = viewInverse.axisZ();

	return true;
}

bool OrthogonalRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	Vector4 origin[4], direction[4];
	calculateRay(rc.getTopRight(), origin[0], direction[0]);
	calculateRay(rc.getTopLeft(), origin[1], direction[1]);
	calculateRay(rc.getBottomLeft(), origin[2], direction[2]);
	calculateRay(rc.getBottomRight(), origin[3], direction[3]);

	Scalar nz(-1e6f);
	Scalar fz(1e6f);

	Vector4 corners[8] =
	{
		origin[0] + direction[0] * nz,
		origin[1] + direction[1] * nz,
		origin[2] + direction[2] * nz,
		origin[3] + direction[3] * nz,
		origin[0] + direction[0] * fz,
		origin[1] + direction[1] * fz,
		origin[2] + direction[2] * fz,
		origin[3] + direction[3] * fz,
	};

	Plane planes[6] =
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

void OrthogonalRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	if (mode == McmMoveXY || mode == McmMoveXZ)
	{
		Vector4 cameraPosition = m_camera->getPosition();
		cameraPosition -= m_camera->getOrientation() * viewDelta.xyz0();
		m_camera->setPosition(cameraPosition);
	}
}

void OrthogonalRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void OrthogonalRenderControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< PropertyColor >(L"Grid");
	m_colorRef = colors->getProperty< PropertyColor >(L"ReferenceEdge");
	m_colorCamera = colors->getProperty< PropertyColor >(L"CameraWire");
}

Matrix44 OrthogonalRenderControl::getProjectionTransform() const
{
	ui::Rect innerRect = m_renderWidget->getInnerRect();
	float ratio = float(innerRect.getWidth()) / innerRect.getHeight();

	world::WorldViewOrtho worldView;
	worldView.width = m_magnification;
	worldView.height = m_magnification / ratio;

	world::WorldRenderView worldRenderView;
	m_worldRenderer->createRenderView(worldView, worldRenderView);

	return worldRenderView.getProjection();
}

Matrix44 OrthogonalRenderControl::getViewTransform() const
{
	return m_camera->getView();
}

void OrthogonalRenderControl::eventButtonDown(ui::Event* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventButtonUp(ui::Event* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventDoubleClick(ui::Event* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventMouseMove(ui::Event* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void OrthogonalRenderControl::eventMouseWheel(ui::Event* event)
{
	int rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();
	
	float delta = m_magnification / 10.0f;

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		m_magnification -= rotation * delta;
	else
		m_magnification += rotation * delta;

	if (m_magnification < c_minMagnification)
		m_magnification = c_minMagnification;

	m_context->raiseCameraMoved();
}

void OrthogonalRenderControl::eventSize(ui::Event* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	updateWorldRenderer();

	m_dirtySize = sz;
}

void OrthogonalRenderControl::eventPaint(ui::Event* event)
{
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

	if (!m_renderView || !m_primitiveRenderer || !m_worldRenderer)
		return;

	// Render world.
	if (m_renderView->begin(render::EtCyclop))
	{
		float tmp[4];
		m_colorClear.getRGBA32F(tmp);

		float ratio = float(m_dirtySize.cx) / m_dirtySize.cy;

		world::WorldViewOrtho worldView;
		worldView.width = m_magnification;
		worldView.height = m_magnification / ratio;

		world::WorldRenderView worldRenderView;
		m_worldRenderer->createRenderView(worldView, worldRenderView);

		Matrix44 view = getViewTransform();

		// Render entities.
		worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
		worldRenderView.setView(view);

		Ref< scene::Scene > sceneInstance = m_context->getScene();
		if (sceneInstance)
		{
			m_worldRenderer->beginBuild();
			m_worldRenderer->build(sceneInstance->getRootEntity());
			m_context->getEntityEventManager()->build(m_worldRenderer);
			m_worldRenderer->endBuild(worldRenderView, 0);
		}

		m_worldRenderer->beginRender(
			0,
			render::EtCyclop,
			Color4f(tmp[0], tmp[1], tmp[2], tmp[3])
		);

		m_worldRenderer->render(
			world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
			0,
			render::EtCyclop
		);

		m_worldRenderer->endRender(0, render::EtCyclop, deltaTime);

		// Draw wire guides.
		m_primitiveRenderer->begin(m_renderView);
		m_primitiveRenderer->setClipDistance(worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushProjection(worldRenderView.getProjection());

		// Render grid.
		if (m_gridEnable)
		{
			Vector4 cameraPosition = -(m_camera->getOrientation().inverse() * m_camera->getPosition().xyz1());

			float hx = worldView.width * 0.5f;
			float hy = worldView.height * 0.5f;

			Vector2 cp(cameraPosition.x(), cameraPosition.y());
			Vector2 vtl(-hx, -hy), vbr(hx, hy);
			Vector2 tl = vtl - cp, br = vbr - cp;

			float lx = floorf(tl.x);
			float rx = ceilf(br.x);
			float ty = floorf(tl.y);
			float by = ceilf(br.y);

			float step = float(
				nearestLog2(int32_t(
					std::max(
						(rx - lx) / 10.0f,
						(by - ty) / 10.0f
					)
				)) - 1
			);

			if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
			{
				float spacing = m_context->getSnapSpacing();
				step = std::max(spacing, step);
			}
			else
				step = std::max(1.0f, step);

			lx -= sign(lx) * std::fmod(abs(lx), step);
			ty -= sign(ty) * std::fmod(abs(ty), step);

			m_primitiveRenderer->pushDepthEnable(false);

			for (float x = lx; x <= rx; x += step)
			{
				float fx = x + cameraPosition.x();

				m_primitiveRenderer->drawLine(
					Vector4(fx, vtl.y, 0.0f, 1.0f),
					Vector4(fx, vbr.y, 0.0f, 1.0f),
					(abs(x) < 0.01f) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			for (float y = ty; y <= by; y += step)
			{
				float fy = y + cameraPosition.y();

				m_primitiveRenderer->drawLine(
					Vector4(vtl.x, fy, 1.0f),
					Vector4(vbr.x, fy, 1.0f),
					(abs(y) < 0.01f) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			m_primitiveRenderer->popDepthEnable();
		}

		m_primitiveRenderer->pushView(view);

		// Draw cameras.
		for (int i = 0; i < 4; ++i)
		{
			const Camera* camera = m_context->getCamera(i);
			if (!camera || !camera->isEnable())
				continue;

			m_primitiveRenderer->pushView(view);
			m_primitiveRenderer->pushWorld(camera->getWorld());
			m_primitiveRenderer->pushDepthEnable(false);

			m_primitiveRenderer->drawWireAabb(
				Vector4::origo(),
				Vector4(0.1f, 0.1f, 0.1f, 0.0f),
				m_colorCamera
			);

			for (int j = 0; j < sizeof_array(c_cameraMeshIndices); j += 2)
			{
				int32_t i1 = c_cameraMeshIndices[j + 0] - 1;
				int32_t i2 = c_cameraMeshIndices[j + 1] - 1;

				const float* v1 = &c_cameraMeshVertices[i1 * 3];
				const float* v2 = &c_cameraMeshVertices[i2 * 3];

				m_primitiveRenderer->drawLine(
					Vector4(v1[0], v1[1], v1[2], 1.0f),
					Vector4(v2[0], v2[1], v2[2], 1.0f),
					m_colorCamera
				);
			}

			m_primitiveRenderer->popDepthEnable();
			m_primitiveRenderer->popWorld();
			m_primitiveRenderer->popView();
		}

		// Draw guides.
		if (m_guideEnable)
		{
			RefArray< EntityAdapter > entityAdapters;
			m_context->getEntities(entityAdapters, SceneEditorContext::GfDefault);

			for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
			{
				if ((*i)->isVisible(true))
					m_context->drawGuide(m_primitiveRenderer, *i);
			}
		}

		// Draw controller guides.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor && m_guideEnable)
			controllerEditor->draw(m_primitiveRenderer);

		// Draw modifier.
		IModifier* modifier = m_context->getModifier();
		if (modifier)
			modifier->draw(m_primitiveRenderer);

		// Draw selection rectangle if non-empty.
		if (m_selectionRectangle.area() > 0)
		{
			ui::Rect innerRect = m_renderWidget->getInnerRect();

			m_primitiveRenderer->pushProjection(orthoLh(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f));
			m_primitiveRenderer->pushView(Matrix44::identity());
			m_primitiveRenderer->pushDepthEnable(false);

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

			m_primitiveRenderer->popDepthEnable();
			m_primitiveRenderer->popView();
			m_primitiveRenderer->popProjection();
		}

		m_primitiveRenderer->end();

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
