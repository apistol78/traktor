#include "Terrain/MaterialMaskFactory.h"
#include "Terrain/MaterialMaskResource.h"
#include "Terrain/MaterialMask.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Reader.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.MaterialMaskFactory", MaterialMaskFactory, resource::IResourceFactory)

MaterialMaskFactory::MaterialMaskFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet MaterialMaskFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< MaterialMask >());
	return typeSet;
}

bool MaterialMaskFactory::isCacheable() const
{
	return true;
}

Object* MaterialMaskFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< MaterialMaskResource > resource = instance->getObject< MaterialMaskResource >();
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	uint32_t size = resource->getSize();
	Ref< MaterialMask > mask = gc_new< MaterialMask >(size);
	Reader(stream).read(mask->m_data, size * size, sizeof(uint8_t));
	stream->close();

	return mask;
}

	}
}
