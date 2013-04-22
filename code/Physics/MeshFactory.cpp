#include "Core/Io/IStream.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Physics/Mesh.h"
#include "Physics/MeshFactory.h"
#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.MeshFactory", MeshFactory, resource::IResourceFactory)

MeshFactory::MeshFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet MeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshResource >());
	return typeSet;
}

const TypeInfoSet MeshFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Mesh >());
	return typeSet;
}

bool MeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > MeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< MeshResource > resource = instance->getObject< MeshResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Mesh > mesh = new Mesh();
	if (!mesh->read(stream))
		mesh = 0;

	stream->close();

	return mesh;
}

	}
}
