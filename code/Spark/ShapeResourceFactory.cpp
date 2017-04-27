/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "Spark/Shape.h"
#include "Spark/ShapeResource.h"
#include "Spark/ShapeResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeResourceFactory", ShapeResourceFactory, resource::IResourceFactory)

ShapeResourceFactory::ShapeResourceFactory(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShapeResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ShapeResource >();
}

const TypeInfoSet ShapeResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Shape >();
}

bool ShapeResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ShapeResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ShapeResource > resource = instance->getObject< ShapeResource >();
	if (resource)
		return resource->create(resourceManager, m_renderSystem, instance);
	else
		return 0;
}

	}
}
