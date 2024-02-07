/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor::world
{

class DecalEvent;
class Entity;
class World;

/*!
 * \ingroup World
 */
class DecalEventInstance : public IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	explicit DecalEventInstance(const DecalEvent* event, World* world, const Transform& Toffset);

	virtual bool update(const UpdateParams& update) override final;

	virtual void cancel(Cancel when) override final;

private:
	Ref< World > m_world;
	Ref< Entity > m_entity;
};

}
