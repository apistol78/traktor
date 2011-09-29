#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Heightfield/MaterialMask.h"
#include "Heightfield/MaterialMaskFactory.h"
#include "Heightfield/MaterialMaskResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.MaterialMaskFactory", MaterialMaskFactory, resource::IResourceFactory)

MaterialMaskFactory::MaterialMaskFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet MaterialMaskFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MaterialMask >());
	return typeSet;
}

bool MaterialMaskFactory::isCacheable() const
{
	return true;
}

Ref< Object > MaterialMaskFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< MaterialMaskResource > resource = instance->getObject< MaterialMaskResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	uint32_t size = resource->getSize();
	Ref< MaterialMask > mask = new MaterialMask(size);
	Reader(stream).read(mask->m_data, size * size, sizeof(uint8_t));
	stream->close();

	return mask;
}

	}
}
