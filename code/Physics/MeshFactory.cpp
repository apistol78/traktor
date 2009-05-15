#include "Physics/MeshFactory.h"
#include "Physics/MeshResource.h"
#include "Physics/Mesh.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.MeshFactory", MeshFactory, resource::ResourceFactory)

MeshFactory::MeshFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet MeshFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Mesh >());
	return typeSet;
}

Object* MeshFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< MeshResource > resource = instance->checkout< MeshResource >(db::CfReadOnly);
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Mesh > mesh = gc_new< Mesh >();
	if (!mesh->read(stream))
		mesh = 0;

	stream->close();

	return mesh;
}

	}
}
