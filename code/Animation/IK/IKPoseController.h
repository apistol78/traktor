#ifndef traktor_animation_IKPoseController_H
#define traktor_animation_IKPoseController_H

#include "Core/Heap/Ref.h"
#include "Animation/PoseController.h"

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
class Body;

	}

	namespace animation
	{

/*! \brief IK pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseController : public PoseController
{
	T_RTTI_CLASS(IKPoseController)

public:
	IKPoseController(physics::PhysicsManager* physicsManager, PoseController* poseController, uint32_t solverIterations);

	void setIgnoreBody(physics::Body* ignoreBody);

	virtual void evaluate(
		float deltaTime,
		const Matrix44& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Matrix44 >& boneTransforms,
		AlignedVector< Matrix44 >& outPoseTransforms,
		bool& outUpdateController
	);

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	);

	inline PoseController* getNeutralPoseController() const { return m_poseController; }

private:
	Ref< physics::PhysicsManager > m_physicsManager;
	Ref< PoseController > m_poseController;
	uint32_t m_solverIterations;
	Ref< physics::Body > m_ignoreBody;
};

	}
}

#endif	// traktor_animation_IKPoseController_H
