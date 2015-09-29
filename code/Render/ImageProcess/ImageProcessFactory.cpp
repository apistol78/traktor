#include "Database/Database.h"
#include "Render/ImageProcess/ImageProcessFactory.h"
#include "Render/ImageProcess/ImageProcessSettings.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessFactory", ImageProcessFactory, resource::IResourceFactory)

ImageProcessFactory::ImageProcessFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet ImageProcessFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ImageProcessSettings >());
	return typeSet;
}

const TypeInfoSet ImageProcessFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ImageProcessSettings >());
	return typeSet;
}

bool ImageProcessFactory::isCacheable() const
{
	return true;
}

Ref< Object > ImageProcessFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< ImageProcessSettings >(guid);
}

	}
}
