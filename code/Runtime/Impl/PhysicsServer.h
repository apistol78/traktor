/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IPhysicsServer.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class PhysicsServer : public IPhysicsServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, const PropertyGroup* settings);

	void destroy();

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float simulationDeltaTime);

	virtual physics::PhysicsManager* getPhysicsManager() override final;

private:
	Ref< physics::PhysicsManager > m_physicsManager;
};

}
