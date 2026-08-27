/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/Transform.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class DisplacementEntityComponentData;

/*! Displacement volume component.
 * \ingroup World
 */
class T_DLLCLASS DisplacementEntityComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit DisplacementEntityComponent(const DisplacementEntityComponentData* data);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	const Vector4& getPosition() const { return m_position; }

	float getRadius() const { return m_radius; }

	float getStrength() const { return m_strength; }

	float getFalloff() const { return m_falloff; }

	float getPress() const { return m_press; }

	void setStrength(float strength) { m_strength = strength; }

private:
	Entity* m_owner = nullptr;
	Vector4 m_position = Vector4::origo();
	float m_radius;
	float m_strength;
	float m_falloff;
	float m_press;
};

}
