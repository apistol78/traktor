/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Flash/Movie.h"
#include "Flash/MovieResourceFactory.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieResourceFactory", MovieResourceFactory, resource::IResourceFactory)

const TypeInfoSet MovieResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< Movie >();
}

const TypeInfoSet MovieResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Movie >();
}

bool MovieResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > MovieResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< Movie >();
}

	}
}
