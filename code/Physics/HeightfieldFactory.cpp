#include "Physics/HeightfieldFactory.h"
#include "Physics/HeightfieldResource.h"
#include "Physics/Heightfield.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.HeightfieldFactory", HeightfieldFactory, resource::IResourceFactory)

HeightfieldFactory::HeightfieldFactory(db::Database* db)
:	m_db(db)
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
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< HeightfieldResource > resource = instance->getObject< HeightfieldResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Heightfield > heightfield = new Heightfield(resource->getSize(), resource->getWorldExtent());
	if (!heightfield->read(stream))
		heightfield = 0;

	stream->close();

	return heightfield;
}

	}
}
