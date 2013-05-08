#ifndef traktor_physics_AxisJointDesc_H
#define traktor_physics_AxisJointDesc_H

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

/*! \brief Axis joint description.
 * \ingroup Physics
 */
class T_DLLCLASS AxisJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	AxisJointDesc();

	void setAnchor(const Vector4& anchor);

	const Vector4& getAnchor() const;

	void setAxis(const Vector4& axis);

	const Vector4& getAxis() const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_anchor;
	Vector4 m_axis;
};

	}
}

#endif	// traktor_physics_AxisJointDesc_H
