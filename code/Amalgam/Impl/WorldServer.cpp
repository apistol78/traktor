#include "Ai/NavMeshEntityFactory.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/AudioServer.h"
#include "Amalgam/Impl/PhysicsServer.h"
#include "Amalgam/Impl/RenderServer.h"
#include "Amalgam/Impl/ScriptServer.h"
#include "Amalgam/Impl/WorldServer.h"
#include "Animation/AnimatedMeshEntityFactory.h"
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
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshEntityRenderer.h"
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
#include "Weather/WeatherEntityFactory.h"
#include "Weather/WeatherEntityRenderer.h"
#include "World/EntityBuilder.h"
#include "World/EntityEventManager.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityResourceFactory.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/DecalEntityRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/TransientEntityRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/PostProcess/PostProcessFactory.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.WorldServer", WorldServer, IWorldServer)

WorldServer::WorldServer()
:	m_worldType(0)
,	m_shadowQuality(world::QuMedium)
,	m_ambientOcclusionQuality(world::QuMedium)
,	m_antiAliasQuality(world::QuMedium)
,	m_particleQuality(world::QuMedium)
,	m_oceanQuality(world::QuMedium)
{
}

bool WorldServer::create(const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer)
{
	std::wstring worldType = settings->getProperty< PropertyString >(L"World.Type");

	m_worldType = TypeInfo::find(worldType);
	if (!m_worldType)
	{
		log::error << L"Unable to create world server; no such type \"" << worldType << L"\"" << Endl;
		return false;
	}

	m_shadowQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", world::QuMedium);
	m_ambientOcclusionQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", world::QuMedium);
	m_antiAliasQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", world::QuMedium);
	m_particleQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ParticleQuality", world::QuMedium);
	m_oceanQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.OceanQuality", world::QuMedium);

	m_renderServer = renderServer;
	m_resourceServer = resourceServer;
	m_entityBuilder = new world::EntityBuilder();

	m_feedbackManager = new spray::FeedbackManager();

	float sprayLod1Distance = c_sprayLodDistances[m_particleQuality][0];
	float sprayLod2Distance = c_sprayLodDistances[m_particleQuality][1];
	m_effectEntityRenderer = new spray::EffectEntityRenderer(m_renderServer->getRenderSystem(), sprayLod1Distance, sprayLod2Distance);

	bool oceanReflectionEnable = bool(m_oceanQuality >= world::QuHigh);
	m_terrainEntityRenderer = new terrain::EntityRenderer(oceanReflectionEnable);

	m_entityRenderers = new world::WorldEntityRenderers();
	m_entityRenderers->add(new world::DecalEntityRenderer(m_renderServer->getRenderSystem()));
	m_entityRenderers->add(new world::GroupEntityRenderer());
	m_entityRenderers->add(new world::LightEntityRenderer());
	m_entityRenderers->add(new world::TransientEntityRenderer());
	m_entityRenderers->add(new mesh::MeshEntityRenderer());
	m_entityRenderers->add(new mesh::InstanceMeshEntityRenderer());
	m_entityRenderers->add(m_effectEntityRenderer);
	m_entityRenderers->add(new animation::ClothEntityRenderer());
	m_entityRenderers->add(new animation::PathEntityRenderer());
	m_entityRenderers->add(new physics::EntityRenderer());
	m_entityRenderers->add(new weather::WeatherEntityRenderer());
	m_entityRenderers->add(m_terrainEntityRenderer);

	int32_t maxEventInstances = settings->getProperty< PropertyInteger >(L"World.MaxEventInstances", 64);
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
	resourceManager->addFactory(new world::PostProcessFactory(database));
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
	m_entityBuilder->addFactory(new animation::ClothEntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new animation::PathEntityFactory());
	m_entityBuilder->addFactory(new ai::NavMeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new mesh::MeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new spray::EffectEntityFactory(resourceManager, m_eventManager, soundPlayer, m_feedbackManager));
	m_entityBuilder->addFactory(new terrain::EntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new weather::WeatherEntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new world::WorldEntityFactory(resourceManager));
}

int32_t WorldServer::reconfigure(const PropertyGroup* settings)
{
	world::Quality shadowQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", world::QuMedium);
	world::Quality ambientOcclusionQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", world::QuMedium);
	world::Quality antiAliasQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", world::QuMedium);
	world::Quality particleQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.ParticleQuality", world::QuMedium);
	world::Quality oceanQuality = (world::Quality)settings->getProperty< PropertyInteger >(L"World.OceanQuality", world::QuMedium);

	// Check if we need to be reconfigured.
	if (
		shadowQuality == m_shadowQuality &&
		ambientOcclusionQuality == m_ambientOcclusionQuality &&
		antiAliasQuality == m_antiAliasQuality &&
		particleQuality == m_particleQuality &&
		oceanQuality == m_oceanQuality
	)
		return CrUnaffected;

	// Adjust in-place systems.
	float sprayLod1Distance = c_sprayLodDistances[m_particleQuality][0];
	float sprayLod2Distance = c_sprayLodDistances[m_particleQuality][1];
	m_effectEntityRenderer->setLodDistances(sprayLod1Distance, sprayLod2Distance);

	bool oceanReflectionEnable = bool(oceanQuality >= world::QuHigh);
	m_terrainEntityRenderer->setOceanDynamicReflectionEnable(oceanReflectionEnable);

	// Save ghost configuration state.
	m_shadowQuality = shadowQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	m_oceanQuality = oceanQuality;

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

Ref< world::IWorldRenderer > WorldServer::createWorldRenderer(
	const world::WorldRenderSettings* worldRenderSettings,
	const world::PostProcessSettings* postProcessSettings
)
{
	world::WorldCreateDesc wcd;

	wcd.worldRenderSettings = worldRenderSettings;
	wcd.postProcessSettings = postProcessSettings;
	wcd.entityRenderers = m_entityRenderers;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.multiSample = m_renderServer->getMultiSample();
	wcd.frameCount = getFrameCount();

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

int32_t WorldServer::getFrameCount() const
{
	return 2;
}

	}
}
