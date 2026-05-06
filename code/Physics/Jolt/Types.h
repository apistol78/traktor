/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace JPH
{

class Constraint;

}

namespace traktor::physics
{

class BodyJolt;
class Joint;

/*!
 * \ingroup Jolt
 */
struct IWorldCallback
{
	virtual ~IWorldCallback() {}

	virtual void destroyBody(BodyJolt* body) = 0;

	virtual void destroyConstraint(Joint* joint, JPH::Constraint* constraint) = 0;

	virtual void insertConstraint(JPH::Constraint* constraint) = 0;

	virtual void removeConstraint(JPH::Constraint* constraint) = 0;
};

}
