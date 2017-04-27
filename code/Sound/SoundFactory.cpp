/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"

namespace traktor
{
	namespace sound
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
		return 0;
}

	}
}
