#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Format.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/CameraRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Itf/IWidget.h"
#include "World/Entity.h"
#include "World/IEntityEventManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Entity/CameraComponent.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_defaultMultiSample = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CameraRenderControl", CameraRenderControl, ISceneRenderControl)

CameraRenderControl::CameraRenderControl()
:	m_imageProcessQuality(world::QuDisabled)
,	m_shadowQuality(world::QuDisabled)
,	m_ambientOcclusionQuality(world::QuDisabled)
,	m_antiAliasQuality(world::QuDisabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool CameraRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT (m_context);

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_multiSample = settings->getProperty< PropertyInteger >(L"Editor.MultiSample", c_defaultMultiSample);

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect))
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

	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &CameraRenderControl::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &CameraRenderControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_timer.start();
	return true;
}

void CameraRenderControl::destroy()
{
	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void CameraRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
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

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	std::wstring worldRendererTypeName = settings->getProperty< PropertyString >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");
	
	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName);
	if (!worldRendererType)
		return;

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &m_worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.imageProcessQuality = m_imageProcessQuality;
	wcd.multiSample = m_multiSample;
	wcd.frameCount = 1;
	wcd.allTargetsPersistent = true;

	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		m_renderView,
		wcd
	))
	{
		m_worldRenderer = worldRenderer;
	}
}

void CameraRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::custom::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void CameraRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	m_imageProcessQuality = imageProcessQuality;
	m_shadowQuality = shadowQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	updateWorldRenderer();
}

bool CameraRenderControl::handleCommand(const ui::Command& command)
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

void CameraRenderControl::update()
{
	m_renderWidget->update();
}

bool CameraRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool CameraRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

bool CameraRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void CameraRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
}

void CameraRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	m_selectionRectangle = rect;
}

void CameraRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_colorClear = settings->getProperty< PropertyColor >(L"Editor.Colors/Background");
	m_colorGrid = settings->getProperty< PropertyColor >(L"Editor.Colors/Grid");
	m_colorRef = settings->getProperty< PropertyColor >(L"Editor.Colors/ReferenceEdge");
	m_invertPanY = settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertPanY");
}

void CameraRenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::Size sz = event->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	safeDestroy(m_worldRenderer);

	m_dirtySize = sz;
}

void CameraRenderControl::eventPaint(ui::PaintEvent* event)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();

	float colorClear[4];
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

	m_colorClear.getRGBA32F(colorClear);

	if (!sceneInstance || !m_renderView || !m_primitiveRenderer)
		return;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();

	/*
	// Get current camera entity.
	if (!m_context->findAdaptersOfType(type_of< world::CameraComponent >(), m_cameraEntities))
		return;

	// Create world render view.
	world::CameraComponent* cameraEntity = mandatory_non_null_type_cast< world::CameraComponent* >(m_cameraEntities[0]->getEntity());
	if (cameraEntity->getCameraType() == world::CtOrthographic)
	{
		m_worldRenderView.setOrthogonal(
			cameraEntity->getWidth(),
			cameraEntity->getHeight(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}
	else // CtPerspective
	{
		ui::Size sz = m_renderWidget->getInnerRect().getSize();

		m_worldRenderView.setPerspective(
			sz.cx,
			sz.cy,
			float(sz.cx) / sz.cy,
			cameraEntity->getFieldOfView(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}

	// Get current transformations.
	Matrix44 projection = m_worldRenderView.getProjection();
	Matrix44 view = cameraEntity->getTransform(0.0f).inverse().toMatrix44();

	// Render world.
	if (m_renderView->begin(render::EtCyclop))
	{
		// Render entities.
		m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
		m_worldRenderView.setView(view);

		Ref< scene::Scene > sceneInstance = m_context->getScene();
		if (sceneInstance)
		{
			// Build frame from scene entities.
			m_worldRenderer->beginBuild();
			m_worldRenderer->build(sceneInstance->getRootEntity());
			m_context->getEntityEventManager()->build(m_worldRenderer);
			m_worldRenderer->endBuild(m_worldRenderView, 0);

			// Set post process parameters from scene instance.
			render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
			if (postProcess)
			{
				for (SmallMap< render::handle_t, resource::Proxy< render::ITexture > >::const_iterator i = sceneInstance->getImageProcessParams().begin(); i != sceneInstance->getImageProcessParams().end(); ++i)
					postProcess->setTextureParameter(i->first, i->second);
			}
		}

		m_worldRenderer->beginRender(
			0,
			render::EtCyclop,
			Color4f(colorClear[0], colorClear[1], colorClear[2], colorClear[3])
		);

		m_worldRenderer->render(
			world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
			0,
			render::EtCyclop
		);

		m_worldRenderer->endRender(0, render::EtCyclop, deltaTime);

		m_primitiveRenderer->end();

		m_renderView->end();
		m_renderView->present();
	}
	*/

	event->consume();
}

	}
}
