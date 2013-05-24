#include "Database/Database.h"
#include "Input/RumbleEffect.h"
#include "Input/RumbleEffectFactory.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RumbleEffectFactory", RumbleEffectFactory, resource::IResourceFactory)

RumbleEffectFactory::RumbleEffectFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet RumbleEffectFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RumbleEffect >());
	return typeSet;
}

const TypeInfoSet RumbleEffectFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RumbleEffect >());
	return typeSet;
}

bool RumbleEffectFactory::isCacheable() const
{
	return true;
}

Ref< Object > RumbleEffectFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< RumbleEffect >(guid);
}

	}
}
