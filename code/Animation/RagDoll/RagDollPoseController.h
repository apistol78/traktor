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

class Body;
class Joint;
class PhysicsManager;

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
		uint32_t collisionMask,
		bool autoDeactivate,
		bool enabled,
		bool fixateBones,
		float limbMass,
		float linearDamping,
		float angularDamping,
		float linearThreshold,
		float angularThreshold,
		IPoseController* trackPoseController,
		float trackLinearTension,
		float trackAngularTension,
		float trackDuration
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& boneTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) T_OVERRIDE T_FINAL;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) T_OVERRIDE T_FINAL;

	void setEnable(bool enable);

	bool isEnable() const;

	const RefArray< physics::Body >& getLimbs() const;

private:
	Ref< IPoseController > m_trackPoseController;
	Scalar m_trackLinearTension;
	Scalar m_trackAngularTension;
	float m_trackDuration;
	RefArray< physics::Body > m_limbs;
	RefArray< physics::Joint > m_joints;
	Transform m_worldTransform;
	bool m_enable;
};

	}
}

#endif	// traktor_animation_RagDollPoseController_H
