/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "World/EntityData.h"
#include "World/EntityResourceFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityResourceFactory", EntityResourceFactory, resource::IResourceFactory)

const TypeInfoSet EntityResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< EntityData >();
}

const TypeInfoSet EntityResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< EntityData >();
}

bool EntityResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > EntityResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< EntityData >();
}

	}
}
