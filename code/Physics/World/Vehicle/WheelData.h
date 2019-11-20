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

namespace traktor
{
	namespace physics
	{

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS WheelData : public ISerializable
{
	T_RTTI_CLASS;

public:
	WheelData();

	bool getSteer() const { return m_steer; }

	bool getDrive() const { return m_drive; }

	float getRadius() const { return m_radius; }

	const Vector4& getAnchor() const { return m_anchor; }

	const Vector4& getAxis() const { return m_axis; }

	const Range< float >& getSuspensionLength() const { return m_suspensionLength; }

	float getSuspensionSpring() const { return m_suspensionSpring; }

	float getSuspensionDamping() const { return m_suspensionDamping; }

	float getRollingFriction() const { return m_rollingFriction; }

	float getSideFriction() const { return m_sideFriction; }

	float getSlipCornerForce() const { return m_slipCornerForce; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_steer;
	bool m_drive;
	float m_radius;
	Vector4 m_anchor;
	Vector4 m_axis;
	Range< float > m_suspensionLength;
	float m_suspensionSpring;
	float m_suspensionDamping;
	float m_rollingFriction;
	float m_sideFriction;
	float m_slipCornerForce;
};

	}
}

