#include "World/PostProcess/PostProcessFactory.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "Database/Database.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessFactory", PostProcessFactory, resource::IResourceFactory)

PostProcessFactory::PostProcessFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet PostProcessFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< PostProcessSettings >());
	return typeSet;
}

Object* PostProcessFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	return m_db->getObjectReadOnly< PostProcessSettings >(guid);
}

	}
}
