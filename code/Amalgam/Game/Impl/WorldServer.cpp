#include "Ai/NavMeshEntityFactory.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Impl/AudioServer.h"
#include "Amalgam/Game/Impl/PhysicsServer.h"
#include "Amalgam/Game/Impl/RenderServer.h"
#include "Amalgam/Game/Impl/ScriptServer.h"
#include "Amalgam/Game/Impl/WorldServer.h"
#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/Boids/BoidsEntityFactory.h"
#include "Animation/Boids/BoidsEntityRenderer.h"
#include "Animation/Cloth/ClothEntityFactory.h"
#include "Animation/Cloth/ClothEntityRenderer.h"
#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "Physics/World/EntityRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/SceneFactory.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectEntityRenderer.h"
#include "Spray/Feedback/FeedbackManager.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainFactory.h"
#include "Weather/WeatherFactory.h"
#include "Weather/WeatherRenderer.h"
#include "World/EntityBuilder.h"
#include "World/EntityEventManager.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityResourceFactory.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/ComponentEntityRenderer.h"
#include "World/Entity/DecalRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/SwitchEntityRenderer.h"
#include "World/Entity/TransientEntityRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/PreLit/WorldRendererPreLit.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const float c_sprayLodDistances[][2] =
{
	{ 0.0f, 0.0f },
	{ 10.0f, 20.0f },
	{ 40.0f, 90.0f },
	{ 60.0f, 140.0f },
	{ 100.0f, 200.0f }
};

const float c_terrainDetailDistances[] =
{
	0.0f,
	10.0f,
	30.0f,
	100.0f,
	200.0f
};

const uint32_t c_terrainSurfaceCacheSizes[] =
{
#if !defined(__PS3__)
	0,
	1024,
	2048,
	4096,
	4096
#else
	0,
	1024,
	1024,
	2048,
	2048
#endif
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.WorldServer", WorldServer, IWorldServer)

WorldServer::WorldServer()
:	m_worldType(0)
,	m_shadowQuality(world::QuMedium)
,	m_ambientOcclusionQuality(world::QuMedium)
,	m_antiAliasQuality(world::QuMedium)
,	m_imageProcessQuality(world::QuMedium)
,	m_particleQuality(world::QuMedium)
,	m_terrainQuality(world::QuMedium)
,	m_oceanQuality(world::QuMedium)
,	m_gamma(2.2f)
,	m_superSample(0)
{
}

bool WorldServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer)
{
	std::wstring worldType = defaultSettings->getProperty< PropertyString >(L"World.Type");

	m_worldType = TypeInfo::find(worldType);
	if (!m_worldType)
	{
		log::error << L"Unable to create world server; no such type \"" << worldType << L"\"" << Endl;
		return false;
	}

	m_shadowQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", world::QuMedium);
	m_ambientOcclusionQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", world::QuMedium);
	m_antiAliasQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", world::QuMedium);
	m_imageProcessQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ImageProcessQuality", world::QuMedium);
	m_particleQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ParticleQuality", world::QuMedium);
	m_terrainQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.TerrainQuality", world::QuMedium);
	m_oceanQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.OceanQuality", world::QuMedium);
	m_gamma = settings->getProperty< PropertyFloat >(L"World.Gamma", 2.2f);
	m_superSample = settings->getProperty< PropertyInteger >(L"World.SuperSample", 0);

	m_renderServer = renderServer;
	m_resourceServer = resourceServer;
	m_entityBuilder = new world::EntityBuilder();

	m_feedbackManager = new spray::FeedbackManager();

	float sprayLod1Distance = c_sprayLodDistances[m_particleQuality][0];
	float sprayLod2Distance = c_sprayLodDistances[m_particleQuality][1];
	m_effectEntityRenderer = new spray::EffectEntityRenderer(m_renderServer->getRenderSystem(), sprayLod1Distance, sprayLod2Distance);

	m_terrainEntityRenderer = new terrain::EntityRenderer(
		c_terrainDetailDistances[m_terrainQuality],
		c_terrainSurfaceCacheSizes[m_terrainQuality],
		bool(m_terrainQuality >= world::QuMedium),
		bool(m_oceanQuality >= world::QuHigh)
	);

	m_entityRenderers = new world::WorldEntityRenderers();
	m_entityRenderers->add(new world::ComponentEntityRenderer());
	m_entityRenderers->add(new world::DecalRenderer(m_renderServer->getRenderSystem()));
	m_entityRenderers->add(new world::GroupEntityRenderer());
	m_entityRenderers->add(new world::LightEntityRenderer());
	m_entityRenderers->add(new world::SwitchEntityRenderer());
	m_entityRenderers->add(new world::TransientEntityRenderer());
	m_entityRenderers->add(new mesh::MeshComponentRenderer());
	m_entityRenderers->add(new mesh::MeshEntityRenderer());
	m_entityRenderers->add(new mesh::InstanceMeshComponentRenderer());
	m_entityRenderers->add(new mesh::InstanceMeshEntityRenderer());
	m_entityRenderers->add(m_effectEntityRenderer);
	m_entityRenderers->add(new animation::BoidsEntityRenderer());
	m_entityRenderers->add(new animation::ClothEntityRenderer());
	m_entityRenderers->add(new animation::PathEntityRenderer());
	m_entityRenderers->add(new physics::EntityRenderer());
	m_entityRenderers->add(new weather::WeatherRenderer());
	m_entityRenderers->add(m_terrainEntityRenderer);

	int32_t maxEventInstances = settings->getProperty< PropertyInteger >(L"World.MaxEventInstances", 512);
	m_eventManager = new world::EntityEventManager(maxEventInstances);

	return true;
}

void WorldServer::destroy()
{
	m_worldType = 0;
	m_resourceServer = 0;
	m_renderServer = 0;
	m_entityBuilder = 0;
	m_entityRenderers = 0;
	m_eventManager = 0;
}

void WorldServer::createResourceFactories(IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new scene::SceneFactory(database, renderSystem, m_entityBuilder));
	resourceManager->addFactory(new terrain::TerrainFactory(database));
	resourceManager->addFactory(new world::EntityEventResourceFactory(database, m_entityBuilder));
	resourceManager->addFactory(new world::EntityResourceFactory(database));
}

void WorldServer::createEntityFactories(IEnvironment* environment)
{
	physics::PhysicsManager* physicsManager = environment->getPhysics() ? environment->getPhysics()->getPhysicsManager() : 0;
	sound::ISoundPlayer* soundPlayer = environment->getAudio() ? environment->getAudio()->getSoundPlayer() : 0;
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	m_entityBuilder->addFactory(new animation::AnimatedMeshEntityFactory(resourceManager, physicsManager));
	m_entityBuilder->addFactory(new animation::BoidsEntityFactory());
	m_entityBuilder->addFactory(new animation::ClothEntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new animation::PathEntityFactory());
	m_entityBuilder->addFactory(new ai::NavMeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new mesh::MeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new spray::EffectEntityFactory(resourceManager, m_eventManager, soundPlayer, m_feedbackManager));
	m_entityBuilder->addFactory(new terrain::EntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new weather::WeatherFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new world::WorldEntityFactory(resourceManager, false));
}

int32_t WorldServer::reconfigure(const PropertyGroup* settings)
{
	world::Quality shadowQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", world::QuMedium);
	world::Quality ambientOcclusionQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", world::QuMedium);
	world::Quality antiAliasQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", world::QuMedium);
	world::Quality imageProcessQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ImageProcessQuality", world::QuMedium);
	world::Quality particleQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ParticleQuality", world::QuMedium);
	world::Quality terrainQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.TerrainQuality", world::QuMedium);
	world::Quality oceanQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.OceanQuality", world::QuMedium);
	float gamma = settings->getProperty< PropertyFloat >(L"World.Gamma", 2.2f);
	int32_t superSample = settings->getProperty< PropertyInteger >(L"World.SuperSample", 0);

	// Check if we need to be reconfigured.
	if (
		shadowQuality == m_shadowQuality &&
		ambientOcclusionQuality == m_ambientOcclusionQuality &&
		antiAliasQuality == m_antiAliasQuality &&
		imageProcessQuality == m_imageProcessQuality &&
		particleQuality == m_particleQuality &&
		terrainQuality == m_terrainQuality &&
		oceanQuality == m_oceanQuality &&
		gamma == m_gamma &&
		superSample == m_superSample
	)
		return CrUnaffected;

	// Adjust in-place systems.
	float sprayLod1Distance = c_sprayLodDistances[m_particleQuality][0];
	float sprayLod2Distance = c_sprayLodDistances[m_particleQuality][1];
	m_effectEntityRenderer->setLodDistances(sprayLod1Distance, sprayLod2Distance);

	m_terrainEntityRenderer->setTerrainDetailDistance(c_terrainDetailDistances[terrainQuality]);
	m_terrainEntityRenderer->setTerrainCacheSize(c_terrainSurfaceCacheSizes[terrainQuality]);
	m_terrainEntityRenderer->setTerrainLayersEnable(bool(terrainQuality >= world::QuMedium));
	m_terrainEntityRenderer->setOceanDynamicReflectionEnable(bool(oceanQuality >= world::QuHigh));

	// Save ghost configuration state.
	m_shadowQuality = shadowQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	m_imageProcessQuality = imageProcessQuality;
	m_terrainQuality = terrainQuality;
	m_oceanQuality = oceanQuality;
	m_gamma = gamma;
	m_superSample = superSample;

	return CrAccepted;
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

const world::IEntityBuilder* WorldServer::getEntityBuilder()
{
	return m_entityBuilder;
}

world::WorldEntityRenderers* WorldServer::getEntityRenderers()
{
	return m_entityRenderers;
}

world::IEntityEventManager* WorldServer::getEntityEventManager()
{
	return m_eventManager;
}

spray::IFeedbackManager* WorldServer::getFeedbackManager()
{
	return m_feedbackManager;
}

Ref< world::IWorldRenderer > WorldServer::createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings)
{
	world::WorldCreateDesc wcd;

	wcd.worldRenderSettings = worldRenderSettings;
	wcd.entityRenderers = m_entityRenderers;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.imageProcessQuality = m_imageProcessQuality;
	wcd.multiSample = m_renderServer->getMultiSample();
	wcd.superSample = m_superSample;
	wcd.frameCount = m_renderServer->getThreadFrameQueueCount();
	wcd.gamma = m_gamma;

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(m_worldType->createInstance());
	if (!worldRenderer)
		return 0;

	if (!worldRenderer->create(
		m_resourceServer->getResourceManager(),
		m_renderServer->getRenderSystem(),
		m_renderServer->getRenderView(),
		wcd
	))
		return 0;

	return worldRenderer;
}

	}
}
