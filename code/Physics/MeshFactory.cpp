#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "Physics/Mesh.h"
#include "Physics/MeshFactory.h"
#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.MeshFactory", MeshFactory, resource::IResourceFactory)

const TypeInfoSet MeshFactory::getResourceTypes() const
{
	return makeTypeInfoSet< MeshResource >();
}

const TypeInfoSet MeshFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Mesh >();
}

bool MeshFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > MeshFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
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
