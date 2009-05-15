#include "Physics/HeightfieldFactory.h"
#include "Physics/HeightfieldResource.h"
#include "Physics/Heightfield.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HeightfieldFactory", HeightfieldFactory, resource::ResourceFactory)

HeightfieldFactory::HeightfieldFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet HeightfieldFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Heightfield >());
	return typeSet;
}

Object* HeightfieldFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< HeightfieldResource > resource = instance->checkout< HeightfieldResource >(db::CfReadOnly);
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Heightfield > heightfield = gc_new< Heightfield >(resource->getSize(), cref(resource->getWorldExtent()));
	if (!heightfield->read(stream))
		heightfield = 0;

	stream->close();

	return heightfield;
}

	}
}
