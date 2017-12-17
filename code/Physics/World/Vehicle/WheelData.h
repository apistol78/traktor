#ifndef traktor_physics_WheelData_H
#define traktor_physics_WheelData_H

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

	float getSuspensionSpringCoeff() const { return m_suspensionSpringCoeff; }

	float getSuspensionDampingCoeff() const { return m_suspensionDampingCoeff; }

	float getRollingFrictionCoeff() const { return m_rollingFrictionCoeff; }

	float getSideFrictionCoeff() const { return m_sideFrictionCoeff; }

	float getSlipCornerForceCoeff() const { return m_slipCornerForceCoeff; }

	virtual void serialize(ISerializer& s);

private:
	bool m_steer;
	bool m_drive;
	float m_radius;
	Vector4 m_anchor;
	Vector4 m_axis;
	Range< float > m_suspensionLength;
	float m_suspensionSpringCoeff;
	float m_suspensionDampingCoeff;
	float m_rollingFrictionCoeff;
	float m_sideFrictionCoeff;
	float m_slipCornerForceCoeff;
};

	}
}

#endif	// traktor_physics_WheelData_H
