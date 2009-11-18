#ifndef traktor_animation_RagDollPoseControllerData_H
#define traktor_animation_RagDollPoseControllerData_H

#include "Animation/IPoseControllerData.h"

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
class T_DLLCLASS RagDollPoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	);

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_animation_RagDollPoseControllerData_H
