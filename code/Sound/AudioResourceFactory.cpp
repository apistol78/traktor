/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Sound/AudioResourceFactory.h"
#include "Sound/IAudioResource.h"
#include "Sound/Sound.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.AudioResourceFactory", 0, AudioResourceFactory, resource::IResourceFactory)

bool AudioResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet AudioResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< IAudioResource >();
}

const TypeInfoSet AudioResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Sound >();
}

bool AudioResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AudioResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const IAudioResource > resource = instance->getObject< IAudioResource >();
	if (resource)
		return resource->createSound(resourceManager, instance);
	else
		return nullptr;
}

}
