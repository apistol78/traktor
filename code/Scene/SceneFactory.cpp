#include "Database/Database.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/SceneResource.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(
	db::Database* database,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder
)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet SceneFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SceneResource >());
	return typeSet;
}

const TypeInfoSet SceneFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Scene >());
	return typeSet;
}

bool SceneFactory::isCacheable() const
{
	return false;
}

Ref< Object > SceneFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< SceneResource > sceneResource = m_database->getObjectReadOnly< SceneResource >(guid);
	if (!sceneResource)
		return 0;

	return sceneResource->createScene(
		resourceManager,
		m_renderSystem,
		m_entityBuilder
	);
}

	}
}
