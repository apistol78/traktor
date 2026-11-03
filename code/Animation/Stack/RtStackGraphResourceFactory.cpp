/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Stack/RtStackGraphResourceFactory.h"

#include "Animation/Stack/RtStackData.h"
#include "Database/Instance.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStackGraphResourceFactory", 0, RtStackGraphResourceFactory, resource::IResourceFactory)

bool RtStackGraphResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RtStackGraphResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< RtStackData >();
}

const TypeInfoSet RtStackGraphResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< RtStackData >();
}

bool RtStackGraphResourceFactory::isCacheable(const TypeInfo& productType) const
{
	// The compiled stack data is immutable and shareable; the mutable runtime
	// RtStack is created per instance (with physics/skeleton) by the caller.
	return true;
}

Ref< Object > RtStackGraphResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< RtStackData >();
}

void RtStackGraphResourceFactory::destroy(Object* resource) const
{
}

}
