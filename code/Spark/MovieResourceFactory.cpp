/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Spark/Movie.h"
#include "Spark/MovieResourceFactory.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.MovieResourceFactory", 0, MovieResourceFactory, resource::IResourceFactory)

bool MovieResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

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
