/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Spark/CharacterData.h"
#include "Spark/CharacterResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterResourceFactory", CharacterResourceFactory, resource::IResourceFactory)

const TypeInfoSet CharacterResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< CharacterData >();
}

const TypeInfoSet CharacterResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< CharacterData >();
}

bool CharacterResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > CharacterResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	return instance->getObject< CharacterData >();
}

	}
}
