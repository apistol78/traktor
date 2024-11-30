/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Resource/FileBundleResourceFactory.h"
#include "Resource/FileBundle.h"

#include "Database/Instance.h"

namespace traktor::resource
{
	T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.FileBundleResourceFactory", 0, FileBundleResourceFactory, resource::IResourceFactory)

		bool FileBundleResourceFactory::initialize(const ObjectStore& objectStore)
	{
		return true;
	}

	const TypeInfoSet FileBundleResourceFactory::getResourceTypes() const
	{
		return makeTypeInfoSet< FileBundle >();
	}

	const TypeInfoSet FileBundleResourceFactory::getProductTypes(const TypeInfo& resourceType) const
	{
		return makeTypeInfoSet< FileBundle >();
	}

	bool FileBundleResourceFactory::isCacheable(const TypeInfo& productType) const
	{
		return true;
	}

	Ref< Object > FileBundleResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
	{
		return instance->getObject< FileBundle >();
	}
}