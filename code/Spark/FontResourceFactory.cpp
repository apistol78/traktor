#include "Database/Database.h"
#include "Database/Instance.h"
#include "Spark/Font.h"
#include "Spark/FontResource.h"
#include "Spark/FontResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.FontResourceFactory", FontResourceFactory, resource::IResourceFactory)

FontResourceFactory::FontResourceFactory(db::Database* db, render::IRenderSystem* renderSystem)
:	m_db(db)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet FontResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FontResource >());
	return typeSet;

}

const TypeInfoSet FontResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Font >());
	return typeSet;
}

bool FontResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > FontResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< FontResource > resource = m_db->getObjectReadOnly< FontResource >(guid);
	if (!resource)
		return 0;

	return resource->create(resourceManager, m_renderSystem);
}

	}
}
