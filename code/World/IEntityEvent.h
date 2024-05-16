/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

/*! Entity event.
 * \ingroup World
 */
class T_DLLCLASS IEntityEvent : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityEventInstance > createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const = 0;
};

}
