/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_IPoseController_H
#define traktor_animation_IPoseController_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"

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

class Pose;
class Skeleton;

/*! \brief Pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IPoseController : public Object
{
	T_RTTI_CLASS;

public:
	struct Velocity
	{
		Vector4 linear;
		Vector4 angular;
	};

	virtual void destroy() = 0;

	virtual void setTransform(const Transform& transform) = 0;

	/*! \brief Evaluate pose throught pose controller.
	 *
	 * \return True if pose is continuous since last evaluation.
	 */
	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) = 0;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) = 0;
};

	}
}

#endif	// traktor_animation_IPoseController_H
