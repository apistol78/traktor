/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/BodyDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

/*! Dynamic rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS DynamicBodyDesc : public BodyDesc
{
	T_RTTI_CLASS;

public:
	DynamicBodyDesc() = default;

	explicit DynamicBodyDesc(ShapeDesc* shape);

	void setMass(float mass);

	float getMass() const;

	void setAutoDeactivate(bool autoDeactivate);

	bool getAutoDeactivate() const;

	void setActive(bool active);

	bool getActive() const;

	void setLinearDamping(float linearDamping);

	float getLinearDamping() const;

	void setAngularDamping(float angularDamping);

	float getAngularDamping() const;

	void setFriction(float friction);

	float getFriction() const;

	void setRestitution(float restitution);

	float getRestitution() const;

	void setLinearThreshold(float linearThreshold);

	float getLinearThreshold() const;

	void setAngularThreshold(float angularThreshold);

	float getAngularThreshold() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_mass = 1.0f;
	bool m_autoDeactivate = true;
	bool m_active = true;
	float m_linearDamping = 0.0f;
	float m_angularDamping = 0.0f;
	float m_friction = 0.75f;
	float m_restitution = 0.0f;
	float m_linearThreshold = 0.8f;
	float m_angularThreshold = 1.0f;
};

}
