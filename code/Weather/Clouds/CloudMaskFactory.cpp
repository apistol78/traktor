#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Weather/Clouds/CloudMask.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudMaskFactory", CloudMaskFactory, resource::IResourceFactory)

CloudMaskFactory::CloudMaskFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet CloudMaskFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudMaskResource >());
	return typeSet;
}

const TypeInfoSet CloudMaskFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudMask >());
	return typeSet;
}

bool CloudMaskFactory::isCacheable() const
{
	return true;
}

Ref< Object > CloudMaskFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< CloudMaskResource > resource = instance->getObject< CloudMaskResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	int32_t size = resource->getSize();

	Ref< CloudMask > mask = new CloudMask(size);
	Reader(stream).read(mask->m_data.ptr(), size * size, sizeof(CloudMask::Sample));
	stream->close();

	return mask;
}

	}
}
