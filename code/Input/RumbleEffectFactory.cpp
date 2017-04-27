/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Input/RumbleEffect.h"
#include "Input/RumbleEffectFactory.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RumbleEffectFactory", RumbleEffectFactory, resource::IResourceFactory)

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
}
