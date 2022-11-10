/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS WobbleComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit WobbleComponent(float magnitude, float rate);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	world::Entity* m_owner = nullptr;
	Transform m_transform = Transform::identity();
	Transform m_local = Transform::identity();
	float m_magnitude;
	float m_rate;
};

	}
}
