#include "Database/Instance.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/SceneResource.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(world::IEntityBuilder* entityBuilder)
:	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet SceneFactory::getResourceTypes() const
{
	return makeTypeInfoSet< SceneResource >();
}

const TypeInfoSet SceneFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Scene >();
}

bool SceneFactory::isCacheable(const TypeInfo& productType) const
{
	return false;
}

Ref< Object > SceneFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< SceneResource > sceneResource = instance->getObject< SceneResource >();
	if (sceneResource)
		return sceneResource->createScene(m_entityBuilder);
	else
		return nullptr;
}

	}
}
