#ifndef traktor_animation_StateNodeController_H
#define traktor_animation_StateNodeController_H

#include "Animation/IPoseController.h"
#include "Animation/Animation/StateContext.h"

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

class StateNode;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeController : public IPoseController
{
	T_RTTI_CLASS;

public:
	StateNodeController(StateNode* node);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	);

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	);

private:
	Ref< StateNode > m_node;
	StateContext m_context;
	bool m_initialized;
};

	}
}

#endif	// traktor_animation_StateNodeController_H
