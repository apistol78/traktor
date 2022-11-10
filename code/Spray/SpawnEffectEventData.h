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
#include "Resource/Id.h"
#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class Effect;

/*! Spawn effect event persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SpawnEffectEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Effect >& getEffect() const { return m_effect; }

	const Transform& getTransform() const { return m_transform; }

	bool getFollow() const { return m_follow; }

	bool getUseRotation() const { return m_useRotation; }

private:
	resource::Id< Effect > m_effect;
	Transform m_transform = Transform::identity();
	bool m_follow = true;
	bool m_useRotation = true;
};

	}
}

