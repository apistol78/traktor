/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/IWorldComponentEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/RenderControls/CameraRenderControl.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/AspectLayout.h"
#include "Ui/Itf/IWidget.h"
#include "World/Entity.h"
#include "World/EntityEventManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::scene
{
	namespace
	{

const int32_t c_defaultMultiSample = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CameraRenderControl", CameraRenderControl, ISceneRenderControl)

CameraRenderControl::CameraRenderControl()
:	m_imageProcessQuality(world::Quality::Disabled)
,	m_shadowQuality(world::Quality::Disabled)
,	m_reflectionsQuality(world::Quality::Disabled)
,	m_motionBlurQuality(world::Quality::Disabled)
,	m_ambientOcclusionQuality(world::Quality::Disabled)
,	m_antiAliasQuality(world::Quality::Disabled)
,	m_gridEnable(true)
,	m_guideEnable(true)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool CameraRenderControl::create(ui::Widget* parent, SceneEditorContext* context, const TypeInfo& worldRendererType)
{
	m_context = context;
	T_ASSERT(m_context);

	m_worldRendererType = &worldRendererType;

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_multiSample = settings->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect, ui::WsNoCanvas))
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
	m_renderGraph = new render::RenderGraph(m_context->getRenderSystem(), m_multiSample);

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
	
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &CameraRenderControl::eventPaint);

	updateSettings();
	return true;
}

void CameraRenderControl::destroy()
{
	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void CameraRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	m_worldRendererType = &worldRendererType;
	safeDestroy(m_worldRenderer);
}

void CameraRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void CameraRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	m_imageProcessQuality = imageProcess;
	m_shadowQuality = shadows;
	m_reflectionsQuality = reflections;
	m_motionBlurQuality = motionBlur;
	m_ambientOcclusionQuality = ambientOcclusion;
	m_antiAliasQuality = antiAlias;
	safeDestroy(m_worldRenderer);
}

void CameraRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
}

void CameraRenderControl::setDebugOverlayAlpha(float alpha, float mip)
{
}

bool CameraRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.PropertiesChanged")
	{
		Ref< scene::Scene > sceneInstance = m_context->getScene();
		if (!sceneInstance)
			return false;

		uint32_t hash = DeepHash(sceneInstance->getWorldRenderSettings()).get();
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

void CameraRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
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

void CameraRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto profile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		profile->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, *m_worldRendererType, entityRenderers);
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
	wcd.quality.reflections = m_reflectionsQuality;
	wcd.quality.shadows = m_shadowQuality;
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

void CameraRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_colorGrid = settings->getProperty< Color4ub >(L"Editor.Colors/Grid");
	m_colorRef = settings->getProperty< Color4ub >(L"Editor.Colors/ReferenceEdge");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
}

void CameraRenderControl::eventPaint(ui::PaintEvent* event)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance || !m_renderView)
		return;

	// Get current camera entity.
	if (!m_context->findAdaptersOfType(type_of< world::CameraComponent >(), m_cameraEntities))
		return;

	auto it = std::find_if(m_cameraEntities.begin(), m_cameraEntities.end(), [](EntityAdapter* entityAdapter) {
		return entityAdapter->isVisible();
	});
	if (it == m_cameraEntities.end())
		return;

	auto cameraEntity = *it;

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

	world::CameraComponent* cameraComponent = cameraEntity->getComponent< world::CameraComponent >();
	T_ASSERT(cameraComponent);

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
	const Matrix44 projection = m_worldRenderView.getProjection();
	const Matrix44 view = cameraEntity->getTransform().inverse().toMatrix44();

	// Build a root entity by gathering entities from containers.
	//Ref< world::GroupComponent > rootGroup = new world::GroupComponent();
	//Ref< world::Entity > rootEntity = new world::Entity();
	//rootEntity->setComponent(rootGroup);

	//m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootGroup->addEntity(entity); });
	//rootGroup->addEntity(sceneInstance->getRootEntity());

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	if (cameraComponent->getProjection() == world::Projection::Orthographic)
	{
		m_worldRenderView.setOrthogonal(
			cameraComponent->getWidth(),
			cameraComponent->getHeight(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}
	else // Projection::Perspective
	{
		m_worldRenderView.setPerspective(
			float(sz.cx),
			float(sz.cy),
			float(sz.cx) / sz.cy,
			cameraComponent->getFieldOfView(),
			worldRenderSettings->viewNearZ,
			worldRenderSettings->viewFarZ
		);
	}
	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, 0, nullptr);

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
