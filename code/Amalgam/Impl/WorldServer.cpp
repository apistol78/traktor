#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Core/Settings/PropertyInteger.h"
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
#include "World/WorldRenderer.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityResourceFactory.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/TransientEntityRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/PostProcess/PostProcessFactory.h"
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
	m_entityBuilder = new world::EntityBuilder();
	m_renderServer = renderServer;
	m_resourceServer = resourceServer;
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
	db::Database* database = environment->getDatabase();

	m_entityBuilder->addFactory(new world::WorldEntityFactory(database));
	m_entityBuilder->addFactory(new mesh::MeshEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new animation::AnimatedMeshEntityFactory(resourceManager, physicsManager));
	m_entityBuilder->addFactory(new animation::PathEntityFactory());
	m_entityBuilder->addFactory(new spray::EffectEntityFactory(resourceManager));
	m_entityBuilder->addFactory(new terrain::EntityFactory(resourceManager, renderSystem));
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

world::IEntityBuilder* WorldServer::getEntityBuilder()
{
	return m_entityBuilder;
}

Ref< world::WorldRenderer > WorldServer::createWorldRenderer(
	const world::WorldRenderSettings* worldRenderSettings,
	const world::WorldEntityRenderers* entityRenderers
)
{
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();

	worldEntityRenderers->add(new world::GroupEntityRenderer());
	worldEntityRenderers->add(new world::LightEntityRenderer());
	worldEntityRenderers->add(new world::TransientEntityRenderer());
	worldEntityRenderers->add(new mesh::MeshEntityRenderer());
	worldEntityRenderers->add(new mesh::InstanceMeshEntityRenderer());
	worldEntityRenderers->add(new spray::EffectEntityRenderer(m_renderServer->getRenderSystem()));
	worldEntityRenderers->add(new animation::PathEntityRenderer());
	worldEntityRenderers->add(new physics::EntityRenderer());
	worldEntityRenderers->add(new weather::WeatherEntityRenderer());
	worldEntityRenderers->add(new terrain::EntityRenderer());

	if (entityRenderers)
	{
		const RefArray< world::IEntityRenderer >& arr = entityRenderers->get();
		for (RefArray< world::IEntityRenderer >::const_iterator i = arr.begin(); i != arr.end(); ++i)
			worldEntityRenderers->add(*i);
	}

	Ref< world::WorldRenderer > worldRenderer = new world::WorldRenderer();
	if (!worldRenderer->create(
		worldRenderSettings,
		worldEntityRenderers,
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
