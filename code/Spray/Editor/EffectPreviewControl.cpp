/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Editor/EffectPreviewControl.h"

#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/Skinned/SkinnedMeshComponentRenderer.h"
#include "Mesh/Static/StaticMeshComponentRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Sound/AudioSystem.h"
#include "Sound/Player/SoundPlayer.h"
#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Editor/LineSourceRenderer.h"
#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectData.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/EffectRenderer.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/EmitterInstanceCPU.h"
#include "Spray/Sources/BoxSourceData.h"
#include "Spray/Sources/ConeSourceData.h"
#include "Spray/Sources/DiscSourceData.h"
#include "Spray/Sources/LineSourceData.h"
#include "Spray/Sources/PointSetSourceData.h"
#include "Spray/Sources/PointSourceData.h"
#include "Spray/Sources/QuadSourceData.h"
#include "Spray/Sources/SphereSourceData.h"
#include "Ui/Application.h"
#include "Ui/Events/ContentChangeEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Weather/Precipitation/PrecipitationRenderer.h"
#include "Weather/Sky/SkyRenderer.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/ProbeRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/World.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"

#include <ctime>

namespace traktor::spray
{
namespace
{

const resource::Id< scene::Scene > c_previewScene(L"{84ADD065-E963-9D4D-A28D-FF44BD616B0F}");

const uint32_t c_initialRandomSeed = 5489UL;
const int c_updateInterval = 30;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectPreviewControl", EffectPreviewControl, render::RenderControl)

EffectPreviewControl::EffectPreviewControl(editor::IEditor* editor)
	: m_editor(editor)
	, m_randomSeed(c_initialRandomSeed)
	, m_timeScale(0.0f)
	, m_extraVelocity(0.0f)
	, m_lastDeltaTime(1.0 / c_updateInterval)
	, m_guideVisible(true)
	, m_gridVisible(false)
	, m_velocityVisible(false)
{
	m_sourceRenderers[&type_of< BoxSourceData >()] = new BoxSourceRenderer();
	m_sourceRenderers[&type_of< ConeSourceData >()] = new ConeSourceRenderer();
	m_sourceRenderers[&type_of< DiscSourceData >()] = new DiscSourceRenderer();
	m_sourceRenderers[&type_of< LineSourceData >()] = new LineSourceRenderer();
	m_sourceRenderers[&type_of< PointSourceData >()] = new PointSourceRenderer();
	m_sourceRenderers[&type_of< PointSetSourceData >()] = new PointSetSourceRenderer();
	m_sourceRenderers[&type_of< QuadSourceData >()] = new QuadSourceRenderer();
	m_sourceRenderers[&type_of< SphereSourceData >()] = new SphereSourceRenderer();
}

bool EffectPreviewControl::create(
	ui::Widget* parent,
	resource::IResourceManager* resourceManager,
	sound::AudioSystem* audioSystem)
{
	if (!render::RenderControl::create(parent, m_editor, resourceManager))
		return false;

	setView({ .position = Vector4(0.0f, -2.0f, 7.0f, 1.0f) });

	m_renderContext = new render::RenderContext(16 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(getRenderSystem(), getMultiSample());

	if ((m_audioSystem = audioSystem) != nullptr)
	{
		m_soundPlayer = new sound::SoundPlayer();
		m_soundPlayer->create(m_audioSystem, nullptr);
	}

	if (!resourceManager->bind(c_previewScene, m_sceneInstance))
		return false;

	m_sceneInstance.consume();

	updateSettings();
	m_timer.reset();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &EffectPreviewControl::eventIdle);
	return true;
}

void EffectPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler(m_idleEventHandler);

	safeDestroy(m_worldRenderer);
	safeDestroy(m_renderGraph);

	safeDestroy(m_soundPlayer);
	m_audioSystem = nullptr;

	render::RenderControl::destroy();
}

void EffectPreviewControl::setEffect(const EffectData* effectData, Effect* effect)
{
	m_effectData = effectData;
	if ((m_effect = effect) != nullptr)
	{
		m_effectEntity = new world::Entity();
		m_effectEntity->setComponent(new EffectComponent(resource::Proxy< Effect >(m_effect)));
	}
	else
		m_effectEntity = nullptr;
}

uint32_t EffectPreviewControl::getEffectLayerPoints(const EffectLayer* effectLayer) const
{
	if (!m_effectEntity)
		return 0;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	auto effectInstance = effectComponent->getEffectInstance();
	if (!effectInstance)
		return 0;

	for (auto layerInstance : effectInstance->getLayerInstances())
	{
		if (layerInstance->getLayer() == effectLayer)
		{
			auto emitterInstance = dynamic_type_cast< const EmitterInstanceCPU* >(layerInstance->getEmitterInstance());
			if (emitterInstance)
				return (uint32_t)emitterInstance->getPoints().size();
			else
				return 0;
		}
	}

	return 0;
}

void EffectPreviewControl::setTotalTime(float totalTime)
{
	if (!m_effectEntity)
		return;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	auto effectInstance = effectComponent->getEffectInstance();
	if (!effectInstance)
		return;

	effectInstance->setTime(totalTime);
}

float EffectPreviewControl::getTotalTime() const
{
	if (!m_effectEntity)
		return 0.0f;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	auto effectInstance = effectComponent->getEffectInstance();
	if (!effectInstance)
		return 0.0f;

	return effectInstance->getTime();
}

void EffectPreviewControl::randomizeSeed()
{
	m_randomSeed = clock();
}

void EffectPreviewControl::syncEffect()
{
	if (!m_effectEntity)
		return;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	float currentTime = 0.0f;

	// Get time from current effect instance.
	{
		auto effectInstance = effectComponent->getEffectInstance();
		if (effectInstance)
			currentTime = effectInstance->getTime();
	}

	Context context;
	context.deltaTime = 0.0f;
	context.extraVelocity = m_extraVelocity;
	context.random = RandomGeometry(m_randomSeed);

	// Create new effect instance.
	Ref< EffectInstance > effectInstance = effectComponent->getEffect()->createInstance();

	// Run emitter until total time is reached.
	const float c_deltaTime = 1.0f / 30.0f;
	for (float T = 0.0f; T < currentTime; T += c_deltaTime)
	{
		const float deltaTime = min(c_deltaTime, currentTime - T);
		context.deltaTime = deltaTime;

		effectInstance->update(context, Transform::identity(), true);
		effectInstance->synchronize();

		m_effectEntity->setTransform(Transform::identity());
	}

	m_effectEntity->setComponent(new EffectComponent(
		effectComponent->getEffect(),
		effectInstance,
		context));

	update();
}

void EffectPreviewControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
}

void EffectPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	if (!m_sceneInstance)
		return;

	const std::wstring worldRendererTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");
	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName.c_str());
	if (!worldRendererType)
		return;

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > entityRenderers = new world::WorldEntityRenderers();
	entityRenderers->add(new mesh::StaticMeshComponentRenderer());
	entityRenderers->add(new mesh::SkinnedMeshComponentRenderer());
	entityRenderers->add(new EffectRenderer(getRenderSystem(), 10000.0f, 10000.0f));
	entityRenderers->add(new weather::PrecipitationRenderer());
	entityRenderers->add(new weather::SkyRenderer());
	entityRenderers->add(new world::ProbeRenderer(
		getResourceManager(),
		getRenderSystem(),
		*worldRendererType));

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	wcd.entityRenderers = entityRenderers;
	wcd.quality.motionBlur = world::Quality::Disabled;
	wcd.quality.shadows = world::Quality::Disabled;
	wcd.quality.reflections = world::Quality::Ultra;
	wcd.quality.ambientOcclusion = world::Quality::Disabled;
	wcd.quality.antiAlias = world::Quality::Disabled;
	wcd.quality.imageProcess = world::Quality::Ultra;
	wcd.quality.irradiance = world::Quality::Ultra;
	wcd.multiSample = 0;
	wcd.hdr = getRenderView()->isHDR();
	wcd.rt = false;

	if (!worldRenderer->create(
			getResourceManager(),
			getRenderSystem(),
			wcd))
	{
		safeDestroy(worldRenderer);
		return;
	}

	m_worldRenderer = worldRenderer;
}

bool EffectPreviewControl::renderFrame()
{
	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		safeDestroy(m_worldRenderer);
		m_sceneInstance.consume();
	}

	if (!m_sceneInstance)
		return false;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return false;
	}

	if (!validateRenderView())
		return false;

	const ui::Size sz = getRenderSize();

	const double time = m_timer.getElapsedTime();
	const double deltaTime = m_timer.getDeltaTime() * 0.9 + m_lastDeltaTime * 0.1;
	m_lastDeltaTime = deltaTime;

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);

	const Matrix44 view = getViewTransform();

	if (m_effectEntity)
	{
		m_effectEntity->setTransform(Transform::identity());
		m_effectEntity->getComponent< EffectComponent >()->setExtraVelocity(m_extraVelocity);

		// Temporarily add effect entity to world.
		m_sceneInstance->getWorld()->addEntity(m_effectEntity);
	}

	// Update scene entities.
	world::UpdateParams update;
	update.totalTime = time;
	update.alternateTime = time;
	update.deltaTime = deltaTime * m_timeScale;
	m_sceneInstance->update(update);

	// Notify owner about effect time being advanced, such as
	// timeline cursor can track playback.
	if (m_timeScale > 0.0f)
	{
		ui::ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(70.0f), // m_fieldOfView),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ);
	m_worldRenderView.setTimes(time, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(m_sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, render::RGTargetSet::Output, nullptr);

	// Remove effect entity from world.
	m_sceneInstance->getWorld()->removeEntity(m_effectEntity);

	// Draw debug wires.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
	rp->setOutput(render::RGTargetSet::Output, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		getPrimitiveRenderer()->begin(0, m_worldRenderView.getProjection());
		getPrimitiveRenderer()->pushView(view);

		if (m_gridVisible)
			drawGrid(getPrimitiveRenderer(), 10.0f, 1.0f);

		if (m_effectEntity && m_guideVisible)
		{
			const Transform transform = m_effectEntity->getTransform();
			getPrimitiveRenderer()->drawWireFrame(transform.toMatrix44(), 1.0f);

			auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
			T_ASSERT(effectComponent != nullptr);

			auto effectInstance = effectComponent->getEffectInstance();
			if (effectInstance)
			{
				for (auto layerInstance : effectInstance->getLayerInstances())
				{
					auto emitterInstance = dynamic_type_cast< const EmitterInstanceCPU* >(layerInstance->getEmitterInstance());
					if (emitterInstance)
					{
						for (const auto& pnt : emitterInstance->getPoints())
						{
							if (pnt.velocity.length() > FUZZY_EPSILON)
							{
								const Vector4 tail = pnt.position + pnt.velocity;
								getPrimitiveRenderer()->drawLine(pnt.position, tail, Color4ub(255, 255, 255, 255));
								getPrimitiveRenderer()->drawArrowHead(tail, tail + pnt.velocity.normalized() * 0.2_simd, 0.8f, Color4ub(255, 255, 255, 255));
							}
							getPrimitiveRenderer()->drawSolidPoint(pnt.position, 6.0f, Color4ub(255, 255, 255, 255));
						}
					}
				}
			}
		}

		if (m_effectData && m_guideVisible)
		{
			for (auto layer : m_effectData->getLayers())
			{
				const EmitterData* emitterData = layer->getEmitter();
				if (!emitterData)
					continue;

				const SourceData* sourceData = emitterData->getSource();
				if (!sourceData)
					continue;

				auto it = m_sourceRenderers.find(&type_of(sourceData));
				if (it != m_sourceRenderers.end())
					it->second->render(getPrimitiveRenderer(), sourceData);
			}
		}

		getPrimitiveRenderer()->end(0);

		auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
		rb->lambda = [&](render::IRenderView* renderView) {
			getPrimitiveRenderer()->render(getRenderView(), 0);
		};
		renderContext->draw(rb);
	});
	m_renderGraph->addPass(rp);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return false;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, sz.cx, sz.cy);

	// Render frame.
	render::IRenderView* renderView = getRenderView();
	if (!renderView->beginFrame())
		return false;

	m_renderContext->render(renderView);
	renderView->endFrame();
	renderView->present();

	// Need to clear all entities from our root group since when our root entity
	// goes out of scope it's automatically destroyed.
	// rootGroup->removeAllEntities();

	return true;
}

void EffectPreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

}
