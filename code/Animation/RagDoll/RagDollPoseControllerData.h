#ifndef traktor_animation_RagDollPoseControllerData_H
#define traktor_animation_RagDollPoseControllerData_H

#include "Animation/IPoseControllerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	RagDollPoseControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	IPoseControllerData* getTrackPoseController() const { return m_trackPoseController; }

private:
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	bool m_autoDeactivate;
	bool m_enabled;
	bool m_fixateBones;
	float m_limbMass;
	float m_linearDamping;
	float m_angularDamping;
	float m_linearThreshold;
	float m_angularThreshold;
	Ref< IPoseControllerData > m_trackPoseController;
	float m_trackLinearTension;
	float m_trackAngularTension;
	float m_trackDuration;
};

	}
}

#endif	// traktor_animation_RagDollPoseControllerData_H
