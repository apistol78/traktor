#ifndef traktor_physics_HingeJointDesc_H
#define traktor_physics_HingeJointDesc_H

#include "Physics/JointDesc.h"
#include "Core/Math/Vector4.h"

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

/*! \brief Hinge joint description.
 * \ingroup Physics
 */
class T_DLLCLASS HingeJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	HingeJointDesc();

	void setAnchor(const Vector4& anchor);

	const Vector4& getAnchor() const;

	void setAxis(const Vector4& axis);

	const Vector4& getAxis() const;

	void setEnableLimits(bool enableLimits);

	bool getEnableLimits() const;

	void setAngles(float minAngle, float maxAngle);

	void getAngles(float& outMinAngle, float& outMaxAngle) const;

	void setAngularOnly(bool angularOnly);

	bool getAngularOnly() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Vector4 m_anchor;
	Vector4 m_axis;
	bool m_enableLimits;
	float m_minAngle;
	float m_maxAngle;
	bool m_angularOnly;
};

	}
}

#endif	// traktor_physics_HingeJointDesc_H
