/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Physics/JointDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Cone twist joint description.
 * \ingroup Physics
 */
class T_DLLCLASS ConeTwistJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	ConeTwistJointDesc();

	void setAnchor(const Vector4& anchor);

	void setConeAxis(const Vector4& coneAxis);

	void setTwistAxis(const Vector4& twistAxis);

	void setConeAngles(float coneAngle1, float coneAngle2);

	void setTwistAngle(float twistAngle);

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getAnchor() const { return m_anchor; }

	const Vector4& getConeAxis() const { return m_coneAxis; }

	const Vector4& getTwistAxis() const { return m_twistAxis; }

	void getConeAngles(float& outConeAngle1, float& outConeAngle2) const { outConeAngle1 = m_coneAngle1; outConeAngle2 = m_coneAngle2; }

	float getTwistAngle() const { return m_twistAngle; }

private:
	Vector4 m_anchor;
	Vector4 m_coneAxis;
	Vector4 m_twistAxis;
	float m_coneAngle1;
	float m_coneAngle2;
	float m_twistAngle;
};

	}
}

