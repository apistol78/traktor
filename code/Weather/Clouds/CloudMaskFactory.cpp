#include "Weather/Clouds/CloudMaskFactory.h"
#include "Weather/Clouds/CloudMaskResource.h"
#include "Weather/Clouds/CloudMask.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Reader.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudMaskFactory", CloudMaskFactory, resource::IResourceFactory)

CloudMaskFactory::CloudMaskFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet CloudMaskFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< CloudMask >());
	return typeSet;
}

bool CloudMaskFactory::isCacheable() const
{
	return true;
}

Object* CloudMaskFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< CloudMaskResource > resource = instance->getObject< CloudMaskResource >();
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	int32_t size = resource->getSize();

	Ref< CloudMask > mask = gc_new< CloudMask >(size);
	Reader(stream).read(mask->m_data.ptr(), size * size, sizeof(CloudMask::Sample));
	stream->close();

	return mask;
}

	}
}
