#pragma once

#include <set>
#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

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

class CollisionSpecification;

	}

	namespace animation
	{

/*! Ragdoll pose evaluation controller data.
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
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const IPoseControllerData* getTrackPoseController() const { return m_trackPoseController; }

private:
	friend class RagDollPoseController;

	std::set< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionMask;
	bool m_autoDeactivate;
	bool m_enabled;
	bool m_fixateJoints;
	float m_limbMass;
	float m_linearDamping;
	float m_angularDamping;
	float m_linearThreshold;
	float m_angularThreshold;
	Ref< const IPoseControllerData > m_trackPoseController;
	float m_trackLinearTension;
	float m_trackAngularTension;
	float m_trackDuration;
};

	}
}

