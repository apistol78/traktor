/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
#include "World/IEntityEventInstance.h"

namespace traktor::world
{

class Entity;
class Entity;

}

namespace traktor::spray
{

class EffectComponent;
class SpawnEffectEvent;

/*! Spawn effect event instance.
 * \ingroup Spray
 */
class SpawnEffectEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	explicit SpawnEffectEventInstance(
		const SpawnEffectEvent* spawnEffect,
		world::Entity* sender,
		const Transform& Toffset,
		EffectComponent* effectComponent
	);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void gather(const std::function< void(world::Entity*) >& fn) const override final;

	virtual void cancel(world::Cancel when) override final;

private:
	const SpawnEffectEvent* m_spawnEffect;
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	Ref< EffectComponent > m_effectComponent;
	Ref< world::Entity > m_effectEntity;
};

}
