#ifndef traktor_animation_IKPoseControllerData_H
#define traktor_animation_IKPoseControllerData_H

#include "Core/Heap/Ref.h"
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

/*! \brief IK pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseControllerData : public PoseControllerData
{
	T_RTTI_CLASS(IKPoseControllerData)

public:
	IKPoseControllerData();

	virtual PoseController* createInstance(physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Matrix44& worldTransform);

	virtual bool serialize(Serializer& s);

private:
	Ref< PoseControllerData > m_neutralPoseController;
	uint32_t m_solverIterations;
};

	}
}

#endif	// traktor_animation_IKPoseControllerData_H
