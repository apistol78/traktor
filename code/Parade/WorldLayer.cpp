#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Parade/WorldLayer.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Scene/Scene.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/TransientEntity.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/IEntitySchema.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.WorldLayer", WorldLayer, Layer)

WorldLayer::WorldLayer(
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< scene::Scene >& scene,
	const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
)
:	Layer(name, scriptContext)
,	m_environment(environment)
,	m_scene(scene)
,	m_entities(entities)
,	m_dynamicEntities(new world::GroupEntity())
,	m_alternateTime(0.0f)
,	m_deltaTime(0.0f)
,	m_fieldOfView(70.0f)
,	m_controllerEnable(true)
{
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< PropertyFloat >(L"World.FieldOfView", 70.0f);
}

void WorldLayer::prepare(Stage* stage)
{
	if (m_scene.changed())
	{
		// If render group already exist then ensure it doesn't contain anything
		// before begin re-created as it will otherwise destroy it's children.
		if (m_renderGroup)
		{
			m_renderGroup->removeAllEntities();
			m_renderGroup = 0;
		}

		// Create render entity group; contain scene root as well as dynamic entities.
		m_renderGroup = new world::GroupEntity();
		m_renderGroup->addEntity(m_scene->getRootEntity());
		m_renderGroup->addEntity(m_dynamicEntities);

		// Scene has been successfully validated; drop existing world renderer if we've been flushed.
		m_worldRenderer = 0;
		m_scene.consume();
	}

	// Re-create world renderer.
	if (!m_worldRenderer)
	{
		createWorldRenderer();
		if (!m_worldRenderer)
			return;

		flushScript();
	}
}

void WorldLayer::update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_worldRenderer)
		return;

	// Issue script update method.
	invokeScriptUpdate(stage, control, info);

	// Update scene controller.
	m_scene->update(
		info.getSimulationTime(),
		info.getSimulationDeltaTime(),
		m_alternateTime,
		m_controllerEnable,
		false
	);

	// Update all entities; calling manually because we have exclusive control
	// of dynamic entities and an explicit render root group.
	world::Entity::UpdateParams up;
	up.totalTime = info.getSimulationTime();
	up.deltaTime = info.getSimulationDeltaTime();
	up.alternateTime = m_alternateTime;
	m_renderGroup->update(up);

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();
}

void WorldLayer::build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_worldRenderer)
		return;

	// Get camera entity and extract view transform.
	world::NullEntity* cameraEntity = m_scene->getEntitySchema()->getEntity< world::NullEntity >(L"Camera");
	if (cameraEntity)
	{
		Transform view = cameraEntity->getTransform(info.getInterval());
		m_worldRenderView.setView(view.inverse().toMatrix44());
	}

	// Build frame through world renderer.
	m_worldRenderView.setTimes(
		info.getStateTime(),
		info.getFrameDeltaTime(),
		info.getInterval()
	);
	m_worldRenderer->build(
		m_worldRenderView,
		m_renderGroup,
		frame
	);

	m_deltaTime = info.getFrameDeltaTime();
}

void WorldLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
{
	if (!m_worldRenderer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	// Render previously built frame.
	m_worldRenderer->render(
		world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap,
		frame,
		eye
	);

	// Render world to off-screen target.
	if (m_worldTarget && m_postProcess)
	{
		renderView->begin(m_worldTarget, 0);

		const Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
		renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);
	}

	// Render world.
	m_worldRenderer->render(
		world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
		frame,
		eye
	);

	// Process world target and blit to frame-buffer.
	if (m_worldTarget && m_postProcess)
	{
		renderView->end();

		world::PostProcessStep::Instance::RenderParams params;
		params.viewFrustum = m_worldRenderView.getViewFrustum();
		params.viewToLight = Matrix44::identity();
		params.view = m_worldRenderView.getView();
		params.projection = m_worldRenderView.getProjection();
		params.deltaTime = m_deltaTime;

		m_postProcess->render(
			renderView,
			m_worldTarget,
			m_worldRenderer->getDepthTargetSet(),
			m_worldRenderer->getShadowMaskTargetSet(),
			params
		);
	}
}

void WorldLayer::leave(Stage* stage)
{
	m_scene.clear();
	m_entities.clear();

	safeDestroy(m_renderGroup);
	safeDestroy(m_dynamicEntities);
	safeDestroy(m_worldRenderer);
	safeDestroy(m_postProcess);
	safeDestroy(m_worldTarget);
}

void WorldLayer::reconfigured(Stage* stage)
{
	createWorldRenderer();
}

world::Entity* WorldLayer::getEntity(const std::wstring& name) const
{
	return m_scene->getEntitySchema()->getEntity(name);
}

RefArray< world::Entity > WorldLayer::getEntities(const std::wstring& name) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(name, entities);
	return entities;
}

RefArray< world::Entity > WorldLayer::getEntitiesOf(const TypeInfo& entityType) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entityType, entities);
	return entities;
}

Ref< world::Entity > WorldLayer::createEntity(const std::wstring& name, world::IEntitySchema* entitySchema)
{
	std::map< std::wstring, resource::Proxy< world::EntityData > >::iterator i = m_entities.find(name);
	if (i == m_entities.end())
		return 0;

	world::IEntityBuilder* entityBuilder = m_environment->getWorld()->getEntityBuilder();
	T_ASSERT (entityBuilder);

	entityBuilder->begin(entitySchema);
	Ref< world::Entity > entity = entityBuilder->create(i->second);
	entityBuilder->end();

	return entity;
}

int32_t WorldLayer::getEntityIndex(const world::Entity* entity) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entities);

	RefArray< world::Entity >::const_iterator i = std::find(entities.begin(), entities.end(), entity);
	if (i == entities.end())
		return -1;

	return std::distance< RefArray< world::Entity >::const_iterator >(entities.begin(), i);
}

world::Entity* WorldLayer::getEntityByIndex(int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(index);
}

void WorldLayer::addEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(entity);
}

void WorldLayer::addTransientEntity(world::Entity* entity, float duration)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(new world::TransientEntity(m_dynamicEntities, entity, duration));
}

void WorldLayer::removeEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->removeEntity(entity);
}

world::IEntitySchema* WorldLayer::getEntitySchema() const
{
	return m_scene->getEntitySchema();
}

void WorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
}

world::PostProcess* WorldLayer::getPostProcess() const
{
	return m_postProcess;
}

bool WorldLayer::getViewPosition(const Vector4& worldPosition, Vector4& outViewPosition) const
{
	outViewPosition = m_worldRenderView.getView() * worldPosition.xyz1();
	return true;
}

bool WorldLayer::getScreenPosition(const Vector4& viewPosition, Vector2& outScreenPosition) const
{
	Vector4 clipPosition = m_worldRenderView.getProjection() * viewPosition.xyz1();
	if (clipPosition.w() <= 0.0f)
		return false;
	clipPosition /= clipPosition.w();
	outScreenPosition = Vector2(clipPosition.x(), clipPosition.y());
	return true;
}

void WorldLayer::setFieldOfView(float fieldOfView)
{
	if (fieldOfView != m_fieldOfView)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		T_ASSERT (renderView);

		// Get render view dimensions.
		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		// Create world view.
		world::WorldViewPerspective worldViewPort;
		worldViewPort.width = width;
		worldViewPort.height = height;
		worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
		worldViewPort.fov = deg2rad(fieldOfView);
		m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);

		// Save field of view value as we must be able to re-create
		// world view if view port dimensions change.
		m_fieldOfView = fieldOfView;
	}
}

float WorldLayer::getFieldOfView() const
{
	return m_fieldOfView;
}

void WorldLayer::setAlternateTime(float alternateTime)
{
	m_alternateTime = alternateTime;
}

float WorldLayer::getAlternateTime() const
{
	return m_alternateTime;
}

void WorldLayer::createWorldRenderer()
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	// Destroy previous instances.
	safeDestroy(m_worldRenderer);
	safeDestroy(m_worldTarget);
	safeDestroy(m_postProcess);

	// Get render view dimensions.
	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create world renderer.
	m_worldRenderer = m_environment->getWorld()->createWorldRenderer(
		*m_scene->getWorldRenderSettings()
	);
	if (!m_worldRenderer)
		return;

	// Create world render view.
	world::WorldViewPerspective worldViewPort;
	worldViewPort.width = width;
	worldViewPort.height = height;
	worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
	worldViewPort.fov = deg2rad(m_fieldOfView);
	m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);

	// Create post frame process.
	const world::PostProcessSettings* postProcessSettings = m_scene->getPostProcessSettings();
	if (postProcessSettings)
	{
		m_postProcess = new world::PostProcess();
		if (m_postProcess->create(
			postProcessSettings,
			m_environment->getResource()->getResourceManager(),
			m_environment->getRender()->getRenderSystem(),
			width,
			height
		))
		{
			m_worldTarget = m_environment->getRender()->createOffscreenTarget(
				m_postProcess->requireHighRange() ? render::TfR11G11B10F : render::TfR8G8B8A8,
				false,
				true
			);
		}
		else
			m_postProcess = 0;
	}
}

	}
}
