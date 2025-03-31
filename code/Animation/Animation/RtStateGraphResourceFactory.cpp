/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateGraphResourceFactory.h"

#include "Animation/Animation/RtStateGraph.h"
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
	return makeTypeInfoSet< RtStateGraph >();
}

const TypeInfoSet RtStateGraphResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet(resourceType);
}

bool RtStateGraphResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return false;
}

Ref< Object > RtStateGraphResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const Object > object = instance->getObject();
	if (const RtStateGraphData* stateGraphData = dynamic_type_cast< const RtStateGraphData* >(object))
		return stateGraphData->createInstance(resourceManager);
	else
		return nullptr;
}

}
