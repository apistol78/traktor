/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/RagDoll/RagDollResourceFactory.h"

#include "Animation/RagDoll/RagDollSkeleton.h"
#include "Database/Instance.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollResourceFactory", 0, RagDollResourceFactory, resource::IResourceFactory)

bool RagDollResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RagDollResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< RagDollSkeleton >();
}

const TypeInfoSet RagDollResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< RagDollSkeleton >();
}

bool RagDollResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > RagDollResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< RagDollSkeleton >();
}

void RagDollResourceFactory::destroy(Object* resource) const
{
}

}
