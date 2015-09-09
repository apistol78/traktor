#include "Database/Database.h"
#include "Database/Instance.h"
#include "Spark/Shape.h"
#include "Spark/ShapeResource.h"
#include "Spark/ShapeResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeResourceFactory", ShapeResourceFactory, resource::IResourceFactory)

ShapeResourceFactory::ShapeResourceFactory(db::Database* db, render::IRenderSystem* renderSystem)
:	m_db(db)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShapeResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShapeResource >());
	return typeSet;

}

const TypeInfoSet ShapeResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Shape >());
	return typeSet;
}

bool ShapeResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > ShapeResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< ShapeResource > resource = instance->getObject< ShapeResource >();
	if (!resource)
		return 0;

	return resource->create(resourceManager, m_renderSystem, instance);
}

	}
}
