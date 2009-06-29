#ifndef traktor_animation_StatePoseController_H
#define traktor_animation_StatePoseController_H

#include <map>
#include "Resource/Proxy.h"
#include "Animation/PoseController.h"
#include "Animation/Animation/StateContext.h"

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

class StateGraph;
class State;
class StateContext;

/*! \brief Animation pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS StatePoseController : public PoseController
{
	T_RTTI_CLASS(StatePoseController)

public:
	StatePoseController(const resource::Proxy< StateGraph >& stateGraph);

	void setCondition(const std::wstring& condition, bool enabled);

	void setTimeFactor(float timeFactor);

	virtual void evaluate(
		float deltaTime,
		const Matrix44& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Matrix44 >& boneTransforms,
		AlignedVector< Matrix44 >& outPoseTransforms,
		bool& outUpdateController
	);

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	);

private:
	resource::Proxy< StateGraph > m_stateGraph;
	Ref< State > m_currentState;
	StateContext m_currentStateContext;
	Ref< State > m_nextState;
	StateContext m_nextStateContext;
	float m_blendState;
	float m_blendDuration;
	std::map< std::wstring, bool > m_conditions;
	float m_timeFactor;
};

	}
}

#endif	// traktor_animation_StatePoseController_H
