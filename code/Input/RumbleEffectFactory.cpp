#include "Input/RumbleEffectFactory.h"
#include "Input/RumbleEffect.h"
#include "Database/Database.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RumbleEffectFactory", RumbleEffectFactory, resource::ResourceFactory)

RumbleEffectFactory::RumbleEffectFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet RumbleEffectFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< RumbleEffect >());
	return typeSet;
}

Object* RumbleEffectFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	return m_db->getObjectReadOnly< RumbleEffect >(guid);
}

	}
}
