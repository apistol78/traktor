#ifndef traktor_animation_RagDollPoseController_H
#define traktor_animation_RagDollPoseController_H

#include "Core/Heap/Ref.h"
#include "Animation/IPoseController.h"

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
class DynamicBody;
class Joint;

	}

	namespace animation
	{

/*! \brief Rag doll pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseController : public IPoseController
{
	T_RTTI_CLASS(RagDollPoseController)

public:
	virtual ~RagDollPoseController();

	bool create(
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		const AlignedVector< Transform >& boneTransforms,
		const AlignedVector< Velocity >& velocities,
		bool initiallyDisabled
	);

	virtual void evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& boneTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	);

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	);

	void enableAllLimbs();

private:
	RefArray< physics::DynamicBody > m_limbs;
	RefArray< physics::Joint > m_joints;
};

	}
}

#endif	// traktor_animation_RagDollPoseController_H
