/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Rotator/WobbleComponent.h"
#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS WobbleComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< WobbleComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_magnitude = 1.0f;
	float m_rate = 1.0f;
};

}
