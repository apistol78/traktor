#ifndef traktor_animation_RagDollPoseController_H
#define traktor_animation_RagDollPoseController_H

#include "Core/RefArray.h"
#include "Animation/IPoseController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	RagDollPoseController();

	virtual ~RagDollPoseController();

	/*! \brief Create rag-doll pose controller.
	 *
	 * \param initiallyDisabled If limbs should be initially disabled.
	 * \param fixateBones Fixate parent bones in world space.
	 */
	bool create(
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		const AlignedVector< Transform >& boneTransforms,
		const AlignedVector< Velocity >& velocities,
		uint32_t collisionGroup,
		bool autoDeactivate,
		bool enabled,
		bool fixateBones,
		float limbMass,
		float linearDamping,
		float angularDamping,
		IPoseController* trackPoseController,
		float trackLinearTension,
		float trackAngularTension
	);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

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

	void setEnable(bool enable);

	bool isEnable() const;

	const RefArray< physics::DynamicBody >& getLimbs() const;

private:
	Ref< IPoseController > m_trackPoseController;
	Scalar m_trackLinearTension;
	Scalar m_trackAngularTension;
	RefArray< physics::DynamicBody > m_limbs;
	RefArray< physics::Joint > m_joints;
	Transform m_worldTransform;
	bool m_enable;
};

	}
}

#endif	// traktor_animation_RagDollPoseController_H
