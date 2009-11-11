#include "Spray/EffectFactory.h"
#include "Spray/Effect.h"
#include "Spray/PointSet.h"
#include "Database/Database.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectFactory", EffectFactory, resource::IResourceFactory)

EffectFactory::EffectFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet EffectFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Effect >());
	typeSet.insert(&type_of< PointSet >());
	return typeSet;
}

bool EffectFactory::isCacheable() const
{
	return true;
}

Ref< Object > EffectFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	if (is_type_a< Effect >(resourceType))
		return m_db->getObjectReadOnly< Effect >(guid);
	else if (is_type_a< PointSet >(resourceType))
		return m_db->getObjectReadOnly< PointSet >(guid);
	else
		return 0;
}

	}
}
