/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Physics/PhysicsFactory.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsFactory", 0, PhysicsFactory, resource::IResourceFactory)

bool PhysicsFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

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
		Ref< const MeshResource > resource = instance->getObject< MeshResource >();
		if (!resource)
			return nullptr;

		Ref< IStream > stream = instance->readData(L"Data");
		if (!stream)
			return nullptr;

		Ref< Mesh > mesh = new Mesh();
		if (!mesh->read(stream))
			mesh = nullptr;

		stream->close();
		return mesh;
	}
	else if (is_type_a< CollisionSpecification >(productType))
	{
		return instance->getObject< CollisionSpecification >();
	}
	else
		return nullptr;
}

}
