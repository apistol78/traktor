/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateGraphResourceFactory.h"

#include "Animation/Animation/RtStateGraphData.h"
#include "Database/Instance.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateGraphResourceFactory", 0, RtStateGraphResourceFactory, resource::IResourceFactory)

bool RtStateGraphResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RtStateGraphResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< RtStateGraphData >();
}

const TypeInfoSet RtStateGraphResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< RtStateGraphData >();
}

bool RtStateGraphResourceFactory::isCacheable(const TypeInfo& productType) const
{
	// The compiled state graph data is immutable and shareable; the mutable runtime
	// RtStateGraph is created per instance (with physics/skeleton) by the caller.
	return true;
}

Ref< Object > RtStateGraphResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< RtStateGraphData >();
}

void RtStateGraphResourceFactory::destroy(Object* resource) const
{
}

}
