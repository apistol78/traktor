#include "Input/RumbleEffectFactory.h"
#include "Input/RumbleEffect.h"
#include "Database/Database.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RumbleEffectFactory", RumbleEffectFactory, resource::IResourceFactory)

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

bool RumbleEffectFactory::isCacheable() const
{
	return true;
}

Ref< Object > RumbleEffectFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	return m_db->getObjectReadOnly< RumbleEffect >(guid);
}

	}
}
