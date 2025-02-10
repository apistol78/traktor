/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class Entity;
class IEntityEventInstance;
class EventManagerComponent;

/*! Entity event interface.
 * \ingroup World
 *
 * An entity event can represent short-lived events in the world, such as sound effects, particles
 * etc. which can be "spawn and forget" by game play logic.
 */
class T_DLLCLASS IEntityEvent : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create an instance of the event.
	 *
	 * \param eventManager Event manager which will control the life time of the instance.
	 * \param sender Which entity issued the event.
	 * \param Toffset Offset from sender's transform, together is where the event should be applied.
	 * \return Entity event instance.
	 */
	virtual Ref< IEntityEventInstance > createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const = 0;
};

}
