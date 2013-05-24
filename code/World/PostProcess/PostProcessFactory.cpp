#include "Database/Database.h"
#include "World/PostProcess/PostProcessFactory.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessFactory", PostProcessFactory, resource::IResourceFactory)

PostProcessFactory::PostProcessFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet PostProcessFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PostProcessSettings >());
	return typeSet;
}

const TypeInfoSet PostProcessFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PostProcessSettings >());
	return typeSet;
}

bool PostProcessFactory::isCacheable() const
{
	return true;
}

Ref< Object > PostProcessFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< PostProcessSettings >(guid);
}

	}
}
