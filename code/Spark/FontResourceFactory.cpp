#include "Database/Instance.h"
#include "Spark/Font.h"
#include "Spark/FontResource.h"
#include "Spark/FontResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.FontResourceFactory", FontResourceFactory, resource::IResourceFactory)

FontResourceFactory::FontResourceFactory(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet FontResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< FontResource >();
}

const TypeInfoSet FontResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Font >();
}

bool FontResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > FontResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< FontResource > resource = instance->getObject< FontResource >();
	if (resource)
		return resource->create(resourceManager, m_renderSystem);
	else
		return 0;
}

	}
}
