/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimationResourceFactory.h"

#include "Animation/Animation/Animation.h"
#include "Animation/Pose.h"
#include "Animation/Skeleton.h"
#include "Database/Instance.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationResourceFactory", 0, AnimationResourceFactory, resource::IResourceFactory)

bool AnimationResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet AnimationResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet<
		Animation,
		Skeleton,
		Pose >();
}

const TypeInfoSet AnimationResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet(resourceType);
}

bool AnimationResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AnimationResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject();
}

}
