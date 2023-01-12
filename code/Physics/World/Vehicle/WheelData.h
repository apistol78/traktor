/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Range.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

/*! Wheel persistent data.
 * \ingroup Physics
 */
class T_DLLCLASS WheelData : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool getSteer() const { return m_steer; }

	bool getDrive() const { return m_drive; }

	float getRadius() const { return m_radius; }

	float getBreakFactor() const { return m_breakFactor; }

	const Vector4& getAnchor() const { return m_anchor; }

	const Vector4& getAxis() const { return m_axis; }

	const Range< float >& getSuspensionLength() const { return m_suspensionLength; }

	float getSuspensionSpring() const { return m_suspensionSpring; }

	float getSuspensionDamping() const { return m_suspensionDamping; }

	float getRollingFriction() const { return m_rollingFriction; }

	/*! Max slip cornering force. */
	float getSlipCornerForce() const { return m_slipCornerForce; }

	/*! Peak slip angle before entering frictional range. */
	float getPeakSlipAngle() const { return m_peakSlipAngle; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_steer = false;
	bool m_drive = false;
	float m_radius = 1.0f;
	float m_breakFactor = 0.0f;
	Vector4 m_anchor = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	Vector4 m_axis = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	Range< float > m_suspensionLength = Range< float >(0.1f, 1.0f);
	float m_suspensionSpring = 5.0f;
	float m_suspensionDamping = 5.0f;
	float m_rollingFriction = 1.0f;
	float m_slipCornerForce = 4.0f;
	float m_peakSlipAngle = deg2rad(8.0f);
};

}
