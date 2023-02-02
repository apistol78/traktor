/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IResourceServer.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::db
{

class Database;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class ResourceServer : public IResourceServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, db::Database* database);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void performCleanup();

	virtual resource::IResourceManager* getResourceManager() override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

}
