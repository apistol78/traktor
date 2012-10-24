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
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainFactory.h"
#include "Weather/WeatherEntityFactory.h"
#include "Weather/WeatherEntityRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/DecalEntityRenderer.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityResourceFactory.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.WorldServer", WorldServer, IWorldServer)

WorldServer::WorldServer()
:	m_shadowQuality(world::WorldRenderSettings::SqNoFilter)
,	m_ambientOcclusionQuality(world::WorldRenderSettings::AoqHigh)
,	m_antiAliasQuality(world::WorldRenderSettings::AaqMedium)
{
}

bool WorldServer::create(const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer)
{
	m_renderServer = renderServer;
	m_resourceServer = resourceServer;

	m_entityBuilder = new world::EntityBuilder();

	float sprayLod1Distance = settings->getProperty< PropertyFloat >(L"World.SprayLod1", 40.0f);
	float sprayLod2Distance = settings->getProperty< PropertyFloat >(L"World.SprayLod2", 90.0f);

	m_entityRenderers = new world::WorldEntityRenderers();
	m_entityRenderers->add(new world::DecalEntityRenderer(m_renderServer->getRenderSystem()));
	m_entityRenderers->add(new world::GroupEntityRenderer());
	m_entityRenderers->add(new world::LightEntityRenderer());
	m_entityRenderers->add(new world::TransientEntityRenderer());
	m_entityRenderers->add(new mesh::MeshEntityRenderer());
	m_entityRenderers->add(new mesh::InstanceMeshEntityRenderer());
	m_entityRenderers->add(new spray::EffectEntityRenderer(m_renderServer->getRenderSystem(), sprayLod1Distance, sprayLod2Distance));
	m_entityRenderers->add(new animation::ClothEntityRenderer());
	m_entityRenderers->add(new animation::PathEntityRenderer());
	m_entityRenderers->add(new physics::EntityRenderer());
	m_entityRenderers->add(new weather::WeatherEntityRenderer());
	m_entityRenderers->add(new terrain::EntityRenderer());

	int32_t shadowQuality = settings->getProperty< PropertyInteger >(L"World.ShadowQuality", 1);
	switch (shadowQuality)
	{
	case 0:	// low
		m_shadowQuality = world::WorldRenderSettings::SqLow;
		break;
	case 1:	// medium
		m_shadowQuality = world::WorldRenderSettings::SqMedium;
		break;
	case 2:	// high
		m_shadowQuality = world::WorldRenderSettings::SqHigh;
		break;
	case 3:	// ultra
		m_shadowQuality = world::WorldRenderSettings::SqHighest;
		break;
	}

	int32_t ambientOcclusionQuality = settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", 2);
	switch (ambientOcclusionQuality)
	{
	case 0:	// low
		m_ambientOcclusionQuality = world::WorldRenderSettings::AoqLow;
		break;
	case 1:	// medium
		m_ambientOcclusionQuality = world::WorldRenderSettings::AoqMedium;
		break;
	case 2:	// high
		m_ambientOcclusionQuality = world::WorldRenderSettings::AoqHigh;
		break;
	case 3:	// ultra
		m_ambientOcclusionQuality = world::WorldRenderSettings::AoqHighest;
		break;
	}

	int32_t antiAliasQuality = settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", 2);
	switch (antiAliasQuality)
	{
	case 0:	// low
		m_antiAliasQuality = world::WorldRenderSettings::AaqDisabled;
		break;
	case 1:	// medium
		m_antiAliasQuality = world::WorldRenderSettings::AaqMedium;
		break;
	case 2:	// high
		m_antiAliasQuality = world::WorldRenderSettings::AaqHigh;
		break;
	case 3:	// ultra
		m_antiAliasQuality = world::WorldRenderSettings::AaqHighest;
		break;
	}

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
	resourceManager->addFactory(new terrain::TerrainFactory(database));
	resourceManager->addFactory(new scene::SceneFactory(database, renderSystem, m_entityBuilder));
}

void WorldServer::createEntityFactories(IEnvironment* environment)
{
	physics::PhysicsManager* physicsManager = environment->getPhysics() ? environment->getPhysics()->getPhysicsManager() : 0;
	sound::ISoundPlayer* soundPlayer = environment->getAudio() ? environment->getAudio()->getSoundPlayer() : 0;
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	m_entityBuilder->addFactory(new world::WorldEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new mesh::MeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new animation::AnimatedMeshEntityFactory(resourceManager, physicsManager));
	m_entityBuilder->addFactory(new animation::ClothEntityFactory(resourceManager, renderSystem, physicsManager));
	m_entityBuilder->addFactory(new animation::PathEntityFactory());
	m_entityBuilder->addFactory(new spray::EffectEntityFactory(resourceManager, soundPlayer));
	m_entityBuilder->addFactory(new terrain::EntityFactory(resourceManager, renderSystem));
	m_entityBuilder->addFactory(new weather::WeatherEntityFactory(resourceManager, renderSystem));
}

int32_t WorldServer::reconfigure(const PropertyGroup* settings)
{
	int32_t shadowQuality = settings->getProperty< PropertyInteger >(L"World.ShadowQuality", 1);
	int32_t ambientOcclusionQuality = settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", 2);
	int32_t antiAliasQuality = settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", 2);
	
	world::WorldRenderSettings::ShadowQuality worldShadowQuality;
	switch (shadowQuality)
	{
	case 0:	// low
		worldShadowQuality = world::WorldRenderSettings::SqLow;
		break;
	case 1:	// medium
		worldShadowQuality = world::WorldRenderSettings::SqMedium;
		break;
	case 2:	// high
		worldShadowQuality = world::WorldRenderSettings::SqHigh;
		break;
	case 3:	// ultra
		worldShadowQuality = world::WorldRenderSettings::SqHighest;
		break;
	default:
		worldShadowQuality = m_shadowQuality;
		break;
	}

	world::WorldRenderSettings::AmbientOcclusionQuality worldAmbientOcclusionQuality;
	switch (ambientOcclusionQuality)
	{
	case 0:	// low
		worldAmbientOcclusionQuality = world::WorldRenderSettings::AoqLow;
		break;
	case 1:	// medium
		worldAmbientOcclusionQuality = world::WorldRenderSettings::AoqMedium;
		break;
	case 2:	// high
		worldAmbientOcclusionQuality = world::WorldRenderSettings::AoqHigh;
		break;
	case 3:	// ultra
		worldAmbientOcclusionQuality = world::WorldRenderSettings::AoqHighest;
		break;
	default:
		worldAmbientOcclusionQuality = m_ambientOcclusionQuality;
		break;
	}

	world::WorldRenderSettings::AntiAliasQuality worldAntiAliasQuality;
	switch (antiAliasQuality)
	{
	case 0:	// low
		worldAntiAliasQuality = world::WorldRenderSettings::AaqDisabled;
		break;
	case 1:	// medium
		worldAntiAliasQuality = world::WorldRenderSettings::AaqMedium;
		break;
	case 2:	// high
		worldAntiAliasQuality = world::WorldRenderSettings::AaqHigh;
		break;
	case 3:	// ultra
		worldAntiAliasQuality = world::WorldRenderSettings::AaqHighest;
		break;
	}

	if (
		worldShadowQuality == m_shadowQuality &&
		worldAmbientOcclusionQuality == m_ambientOcclusionQuality &&
		worldAntiAliasQuality == m_antiAliasQuality
	)
		return CrUnaffected;

	m_shadowQuality = worldShadowQuality;
	m_ambientOcclusionQuality = worldAmbientOcclusionQuality;
	m_antiAliasQuality = worldAntiAliasQuality;

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

world::IEntityBuilder* WorldServer::getEntityBuilder()
{
	return m_entityBuilder;
}

world::WorldEntityRenderers* WorldServer::getEntityRenderers()
{
	return m_entityRenderers;
}

Ref< world::IWorldRenderer > WorldServer::createWorldRenderer(
	const world::WorldRenderSettings* worldRenderSettings,
	const world::PostProcessSettings* postProcessSettings
)
{
	Ref< world::WorldRenderSettings > settings;
	
	if (worldRenderSettings)
		settings = new world::WorldRenderSettings(*worldRenderSettings);
	else
		settings = new world::WorldRenderSettings();

	settings->shadowsQuality = m_shadowQuality;
	settings->ambientOcclusionQuality = m_ambientOcclusionQuality;
	settings->antiAliasQuality = m_antiAliasQuality;

	Ref< world::IWorldRenderer > worldRenderer;
	if (settings->renderType == world::WorldRenderSettings::RtForward)
		worldRenderer = new world::WorldRendererForward();
	else if (settings->renderType == world::WorldRenderSettings::RtPreLit)
		worldRenderer = new world::WorldRendererPreLit();

	if (!worldRenderer)
		return 0;

	if (!worldRenderer->create(
		settings,
		postProcessSettings,
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
