/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor::physics
{

class BodyJolt;
class Joint;

/*!
 * \ingroup Physics
 */
struct IWorldCallback
{
	virtual ~IWorldCallback() {}

	virtual void destroyBody(BodyJolt* body) = 0;
};

}
