/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundFactory", SoundFactory, resource::IResourceFactory)

const TypeInfoSet SoundFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ISoundResource >();
}

const TypeInfoSet SoundFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Sound >();
}

bool SoundFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > SoundFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ISoundResource > resource = instance->getObject< ISoundResource >();
	if (resource)
		return resource->createSound(resourceManager, instance);
	else
		return nullptr;
}

}
