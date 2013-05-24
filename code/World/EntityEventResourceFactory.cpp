#include "Database/Database.h"
#include "World/EntityEventResourceFactory.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventResourceFactory", EntityEventResourceFactory, resource::IResourceFactory)

EntityEventResourceFactory::EntityEventResourceFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet EntityEventResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IEntityEventData >());
	return typeSet;
}

const TypeInfoSet EntityEventResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IEntityEventData >());
	return typeSet;
}

bool EntityEventResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > EntityEventResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< IEntityEventData >(guid);
}

	}
}
