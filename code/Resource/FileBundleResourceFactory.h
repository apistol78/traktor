/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{
    class Database;
    class Instance;
}

namespace traktor::resource
{
	/*! FileBundle resource factory.
	 * \ingroup Resource
	 */
	class IResourceManager;

	class T_DLLCLASS FileBundleResourceFactory : public resource::IResourceFactory
	{
		T_RTTI_CLASS;

	public:
		virtual bool initialize(const ObjectStore& objectStore) override final;

		virtual const TypeInfoSet getResourceTypes() const override final;

		virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

		virtual bool isCacheable(const TypeInfo& productType) const override final;

		virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;
	};
}