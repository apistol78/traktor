/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Transform.h"
#include "Core/Thread/Semaphore.h"
#include "World/IWorldComponent.h"
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
class IEntityEvent;
class IEntityEventInstance;

/*!
 * \ingroup World
 */
class T_DLLCLASS EventManagerComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit EventManagerComponent(uint32_t maxEventsInstances);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	IEntityEventInstance* raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset);

	void cancelAll(Cancel when);

private:
	uint32_t m_maxEventInstances;
	RefArray< IEntityEventInstance > m_eventInstances;
	mutable Semaphore m_lock;
};

}
