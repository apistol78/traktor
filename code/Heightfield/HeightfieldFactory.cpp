#include <limits>
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/HeightfieldResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldFactory", HeightfieldFactory, resource::IResourceFactory)

HeightfieldFactory::HeightfieldFactory(db::Database* database)
:	m_database(database)
{
}

const TypeInfoSet HeightfieldFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Heightfield >());
	return typeSet;
}

bool HeightfieldFactory::isCacheable() const
{
	return true;
}

Ref< Object > HeightfieldFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
		return 0;

	Ref< HeightfieldResource > resource = instance->getObject< HeightfieldResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Heightfield > heightfield = new Heightfield(
		resource->getSize(),
		resource->getWorldExtent()
	);

	uint32_t size = resource->getSize();
	height_t* heights = const_cast< height_t* >(heightfield->getHeights());
	T_ASSERT_M (heights, L"No heights in heightfield");
	Reader(stream).read(heights, size * size, sizeof(height_t));
	stream->close();

	return heightfield;
}

	}
}
