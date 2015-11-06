#ifndef traktor_physics_FixedJointDesc_H
#define traktor_physics_FixedJointDesc_H

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

/*! \brief Fixed joint description.
 * \ingroup Physics
 */
class T_DLLCLASS FixedJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_FixedJointDesc_H
