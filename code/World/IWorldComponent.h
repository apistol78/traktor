/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class World;

/*! World component.
 * \ingroup World
 */
class T_DLLCLASS IWorldComponent : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! Update component
	 * \param world World instance.
	 * \param update Update information.
	 */
	virtual void update(World* world, const UpdateParams& update) = 0;
};

}
