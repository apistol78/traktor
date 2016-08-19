#ifndef traktor_animation_SimpleAnimationController_H
#define traktor_animation_SimpleAnimationController_H

#include <map>
#include "Animation/IPoseController.h"
#include "Animation/Animation/StateContext.h"
#include "Resource/Proxy.h"

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

class Animation;
class StateContext;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS SimpleAnimationController : public IPoseController
{
	T_RTTI_CLASS;

public:
	SimpleAnimationController(const resource::Proxy< Animation >& animation, bool linearInterpolation);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) T_OVERRIDE T_FINAL;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) T_OVERRIDE T_FINAL;

private:
	resource::Proxy< Animation > m_animation;
	bool m_linearInterpolation;
	float m_time;
	int32_t m_indexHint;
};

	}
}

#endif	// traktor_animation_SimpleAnimationController_H
