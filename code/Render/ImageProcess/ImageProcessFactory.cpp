/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Render/ImageProcess/ImageProcessFactory.h"
#include "Render/ImageProcess/ImageProcessSettings.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessFactory", ImageProcessFactory, resource::IResourceFactory)

const TypeInfoSet ImageProcessFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ImageProcessSettings >();
}

const TypeInfoSet ImageProcessFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ImageProcessSettings >();
}

bool ImageProcessFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ImageProcessFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< ImageProcessSettings >();
}

	}
}
