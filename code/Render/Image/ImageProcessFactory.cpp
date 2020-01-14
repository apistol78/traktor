#include "Database/Instance.h"
#include "Render/Image/ImageProcessFactory.h"
#include "Render/Image/ImageProcessData.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessFactory", ImageProcessFactory, resource::IResourceFactory)

const TypeInfoSet ImageProcessFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ImageProcessData >();
}

const TypeInfoSet ImageProcessFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ImageProcessData >();
}

bool ImageProcessFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ImageProcessFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< ImageProcessData >();
}

	}
}
