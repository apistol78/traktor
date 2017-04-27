/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieResourceFactory.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieResourceFactory", FlashMovieResourceFactory, resource::IResourceFactory)

const TypeInfoSet FlashMovieResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< FlashMovie >();
}

const TypeInfoSet FlashMovieResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< FlashMovie >();
}

bool FlashMovieResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > FlashMovieResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< FlashMovie >();
}

	}
}
