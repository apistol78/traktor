/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/Cloth.h"
#include "Animation/Cloth/ClothResourceFactory.h"
#include "Database/Instance.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothResourceFactory", 0, ClothResourceFactory, resource::IResourceFactory)

bool ClothResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet ClothResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< Cloth >();
}

const TypeInfoSet ClothResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Cloth >();
}

bool ClothResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ClothResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< Cloth >();
}

}
