/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
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

class Entity;

/*! Entity event instance.
 * \ingroup World
 */
class T_DLLCLASS IEntityEventInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool update(const UpdateParams& update) = 0;

	virtual void gather(const std::function< void(Entity*) >& fn) const = 0;

	virtual void cancel(Cancel when) = 0;
};

}
