#ifndef traktor_physics_Hinge2JointDesc_H
#define traktor_physics_Hinge2JointDesc_H

#include "Physics/JointDesc.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief 2 axis hinge joint description.
 * \ingroup Physics
 */
class T_DLLCLASS Hinge2JointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	Hinge2JointDesc();

	void setAnchor(const Vector4& anchor);

	const Vector4& getAnchor() const;

	void setAxis1(const Vector4& axis);

	const Vector4& getAxis1() const;

	void setAxis2(const Vector4& axis);

	const Vector4& getAxis2() const;

	void setLowStop(float lowStop);

	float getLowStop() const;

	void setHighStop(float highStop);

	float getHighStop() const;

	void setSuspensionEnable(bool suspensionEnable);

	bool getSuspensionEnable() const;

	void setSuspensionDamping(float suspensionDamping);

	float getSuspensionDamping() const;

	void setSuspensionStiffness(float suspensionStiffness);

	float getSuspensionStiffness() const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_anchor;
	Vector4 m_axis1;
	Vector4 m_axis2;
	float m_lowStop;
	float m_highStop;
	bool m_suspensionEnable;
	float m_suspensionDamping;
	float m_suspensionStiffness;
};

	}
}

#endif	// traktor_physics_Hinge2JointDesc_H
