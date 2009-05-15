#include "Spray/EffectFactory.h"
#include "Spray/Effect.h"
#include "Database/Database.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectFactory", EffectFactory, resource::ResourceFactory)

EffectFactory::EffectFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet EffectFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Effect >());
	return typeSet;
}

Object* EffectFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	return m_db->getObjectReadOnly< Effect >(guid);
}

	}
}
