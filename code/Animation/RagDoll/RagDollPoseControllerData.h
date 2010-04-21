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
	);

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_collisionGroup;
	bool m_autoDeactivate;
	bool m_enabled;
	bool m_fixateBones;
	float m_limbMass;
	float m_linearDamping;
	float m_angularDamping;
};

	}
}

#endif	// traktor_animation_RagDollPoseControllerData_H
