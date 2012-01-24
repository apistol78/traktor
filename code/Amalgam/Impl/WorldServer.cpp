#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/Cloth/ClothEntityFactory.h"
#include "Animation/Cloth/ClothEntityRenderer.h"
#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "Physics/World/EntityRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/SceneFactory.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectEntityRenderer.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Weather/WeatherEntityFactory.h"
#include "Weather/WeatherEntityRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityResourceFactory.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/TransientEntityRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/PostProcess/PostProcessFactory.h"
#include "World/PreLit/WorldRendererPreLit.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/AudioServer.h"
#include "Amalgam/Impl/PhysicsServer.h"
#include "Amalgam/Impl/RenderServer.h"
#include "Amalgam/Impl/ScriptServer.h"
#include "Amalgam/Impl/WorldServer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.WorldServer", WorldServer, IWorldServer)

bool WorldServer::create(const Settings* settings, IRenderServer* renderServer, IResourceServer* resourceServer)
{
	m_renderServer = renderServer;
	m_resourceServer = resourceServer;

	m_entityBuilder = new world::EntityBuilder();

	m_entityRenderers = new world::WorldEntityRenderers();
	m_entityRenderers->add(new world::GroupEntityRenderer());
	m_entityRenderers->add(new world::LightEntityRenderer());
	m_entityRenderers->add(new world::TransientEntityRenderer());
	m_entityRenderers->add(new mesh::MeshEntityRenderer());
	m_entityRenderers->add(new mesh::InstanceMeshEntityRenderer());
	m_entityRenderers->add(new spray::EffectEntityRenderer(m_renderServer->getRenderSystem()));
	m_entityRenderers->add(new animation::ClothEntityRenderer());
	m_entityRenderers->add(new animation::PathEntityRenderer());
	m_entityRenderers->add(new physics::EntityRenderer());
	m_entityRenderers->add(new weather::WeatherEntityRenderer());
	m_entityRenderers->add(new terrain::EntityRenderer());

	return true;
}

void WorldServer::destroy()
{
	m_resourceServer = 0;
	m_renderServer = 0;
	m_entityBuilder = 0;
}

void WorldServer::createResourceFactories(IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new world::PostProcessFactory(database));
	resourceManager->addFactory(new world::EntityResourceFactory(database));

	int32_t shadowQuality = environment->getSettings()->getProperty< PropertyInteger >(L"World.ShadowQuality", world::WorldRenderSettings::SqHigh);
	m_sceneFactory = new scene::SceneFactory(
		database,
		renderSystem,
		m_entityBuilder,
		(world::WorldRenderSettings::ShadowQuality)shadowQuality
	);

	resourceManager->addFactory(m_sceneFactory);
}

void WorldServer::createEntityFactories(IEnvironment* environment)
{
	physics::PhysicsManager* physicsManager = environment->getPhysics()->getPhysicsManager();
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	m_entityBuilder->addFactory(new world::WorldEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new mesh::MeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new animation::AnimatedMeshEntityFactory(resourceManager, physicsManager));
	m_entityBuilder->addFactory(new animation::ClothEntityFactory(resourceManager, renderSystem, physicsManager));
	m_entityBuilder->addFactory(new animation::PathEntityFactory());
	m_entityBuilder->addFactory(new spray::EffectEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new terrain::EntityFactory(resourceManager, renderSystem, false));
	m_entityBuilder->addFactory(new weather::WeatherEntityFactory(resourceManager, renderSystem));
}

int32_t WorldServer::reconfigure(const Settings* settings)
{
	int32_t shadowQuality = settings->getProperty< PropertyInteger >(L"World.ShadowQuality", world::WorldRenderSettings::SqHigh);
	if (shadowQuality != m_sceneFactory->getShadowQuality())
	{
		m_sceneFactory->setShadowQuality((world::WorldRenderSettings::ShadowQuality)shadowQuality);
		return CrAccepted | CrFlushResources;
	}
	return CrUnaffected;
}

void WorldServer::addEntityFactory(world::IEntityFactory* entityFactory)
{
	m_entityBuilder->addFactory(entityFactory);
}

void WorldServer::removeEntityFactory(world::IEntityFactory* entityFactory)
{
	m_entityBuilder->removeFactory(entityFactory);
}

void WorldServer::addEntityRenderer(world::IEntityRenderer* entityRenderer)
{
	m_entityRenderers->add(entityRenderer);
}

void WorldServer::removeEntityRenderer(world::IEntityRenderer* entityRenderer)
{
	m_entityRenderers->remove(entityRenderer);
}

world::IEntityBuilder* WorldServer::getEntityBuilder()
{
	return m_entityBuilder;
}

world::WorldEntityRenderers* WorldServer::getEntityRenderers()
{
	return m_entityRenderers;
}

Ref< world::IWorldRenderer > WorldServer::createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings)
{
	T_ASSERT (worldRenderSettings);

	Ref< world::IWorldRenderer > worldRenderer;

	if (worldRenderSettings->renderType == world::WorldRenderSettings::RtForward)
		worldRenderer = new world::WorldRendererForward();
	else if (worldRenderSettings->renderType == world::WorldRenderSettings::RtPreLit)
		worldRenderer = new world::WorldRendererPreLit();

	if (!worldRenderer)
		return 0;

	if (!worldRenderer->create(
		*worldRenderSettings,
		m_entityRenderers,
		m_resourceServer->getResourceManager(),
		m_renderServer->getRenderSystem(),
		m_renderServer->getRenderView(),
		m_renderServer->getMultiSample(),
		getFrameCount()
	))
		return 0;

	return worldRenderer;
}

int32_t WorldServer::getFrameCount() const
{
	return 3;
}

	}
}
