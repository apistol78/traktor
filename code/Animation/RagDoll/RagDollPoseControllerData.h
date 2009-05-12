#ifndef traktor_animation_RagDollPoseControllerData_H
#define traktor_animation_RagDollPoseControllerData_H

#include "Animation/PoseControllerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseControllerData : public PoseControllerData
{
	T_RTTI_CLASS(RagDollPoseControllerData)

public:
	virtual PoseController* createInstance(physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Matrix44& worldTransform);

	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_animation_RagDollPoseControllerData_H
