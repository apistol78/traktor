#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Physics/PhysicsFactory.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsFactory", PhysicsFactory, resource::IResourceFactory)

const TypeInfoSet PhysicsFactory::getResourceTypes() const
{
	return makeTypeInfoSet< MeshResource, CollisionSpecification >();
}

const TypeInfoSet PhysicsFactory::getProductTypes(const TypeInfo& resourceType) const
{
	if (is_type_a< MeshResource >(resourceType))
		return makeTypeInfoSet< Mesh >();
	else if (is_type_a< CollisionSpecification >(resourceType))
		return makeTypeInfoSet< CollisionSpecification >();
	else
		return TypeInfoSet();
}

bool PhysicsFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > PhysicsFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	if (is_type_a< Mesh >(productType))
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
	else if (is_type_a< CollisionSpecification >(productType))
	{
		return instance->getObject< CollisionSpecification >();
	}
	else
		return 0;
}

	}
}
