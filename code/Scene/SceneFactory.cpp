#include "Database/Database.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/SceneResource.h"
#include "World/Entity/EntityManager.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(
	db::Database* database,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder,
	world::WorldRenderSettings::ShadowQuality shadowQuality
)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_entityBuilder(entityBuilder)
,	m_shadowQuality(shadowQuality)
{
}

void SceneFactory::setShadowQuality(world::WorldRenderSettings::ShadowQuality shadowQuality)
{
	m_shadowQuality = shadowQuality;
}

const TypeInfoSet SceneFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Scene >());
	return typeSet;
}

bool SceneFactory::isCacheable() const
{
	return true;
}

Ref< Object > SceneFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< SceneResource > sceneResource = m_database->getObjectReadOnly< SceneResource >(guid);
	if (!sceneResource)
		return 0;

	Ref< world::IEntityManager > entityManager = new world::EntityManager();
	return sceneResource->createScene(
		resourceManager,
		m_renderSystem,
		m_entityBuilder,
		entityManager,
		m_shadowQuality
	);
}

	}
}
