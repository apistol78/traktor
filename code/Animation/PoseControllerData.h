#ifndef traktor_animation_PoseControllerData_H
#define traktor_animation_PoseControllerData_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;

	}

	namespace animation
	{

class PoseController;
class Skeleton;

/*! \brief Pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS PoseControllerData : public Serializable
{
	T_RTTI_CLASS(PoseControllerData)

public:
	virtual PoseController* createInstance(physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Matrix44& worldTransform) = 0;
};

	}
}

#endif	// traktor_animation_PoseControllerData_H
