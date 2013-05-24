#include "Database/Database.h"
#include "World/EntityData.h"
#include "World/EntityResourceFactory.h"

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
	type_of< EntityData >().findAllOf(typeSet);
	return typeSet;
}

const TypeInfoSet EntityResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< EntityData >().findAllOf(typeSet);
	return typeSet;
}

bool EntityResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > EntityResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< EntityData >(guid);
}

	}
}
