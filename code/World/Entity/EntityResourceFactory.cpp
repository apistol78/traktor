#include "Database/Database.h"
#include "World/Entity/EntityResourceFactory.h"
#include "World/Entity/SpatialEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityResourceFactory", EntityResourceFactory, resource::IResourceFactory)

EntityResourceFactory::EntityResourceFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet EntityResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EntityData >());
	typeSet.insert(&type_of< SpatialEntityData >());
	return typeSet;
}

bool EntityResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > EntityResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	return m_db->getObjectReadOnly< EntityData >(guid);
}

	}
}
