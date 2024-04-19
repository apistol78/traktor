/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Input/RumbleEffect.h"
#include "Input/RumbleEffectFactory.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.RumbleEffectFactory", 0, RumbleEffectFactory, resource::IResourceFactory)

bool RumbleEffectFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RumbleEffectFactory::getResourceTypes() const
{
	return makeTypeInfoSet< RumbleEffect >();
}

const TypeInfoSet RumbleEffectFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< RumbleEffect >();
}

bool RumbleEffectFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > RumbleEffectFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< RumbleEffect >();
}

}
