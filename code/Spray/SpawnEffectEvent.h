/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor::spray
{

class Effect;

/*! Spawn effect event.
 * \ingroup Spray
 */
class SpawnEffectEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	explicit SpawnEffectEvent(
		const resource::Proxy< Effect >& effect,
		const Transform& transform,
		bool follow,
		bool useRotation
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::EventManagerComponent* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	friend class SpawnEffectEventInstance;

	resource::Proxy< Effect > m_effect;
	Transform m_transform;
	bool m_follow;
	bool m_useRotation;
};

}
