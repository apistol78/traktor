#include "Scene/SceneFactory.h"
#include "Scene/SceneAsset.h"
#include "Scene/Scene.h"
#include "World/Entity/EntityManager.h"
#include "Database/Database.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(db::Database* database, render::IRenderSystem* renderSystem, world::IEntityBuilder* entityBuilder)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_entityBuilder(entityBuilder)
{
}

const TypeSet SceneFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Scene >());
	return typeSet;
}

bool SceneFactory::isCacheable() const
{
	return true;
}

Object* SceneFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< SceneAsset > asset = m_database->getObjectReadOnly< SceneAsset >(guid);
	if (!asset)
		return 0;

	Ref< world::IEntityManager > entityManager = gc_new< world::EntityManager >();
	return asset->createScene(resourceManager, m_renderSystem, m_entityBuilder, entityManager);
}

	}
}
