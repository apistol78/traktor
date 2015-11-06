#ifndef traktor_animation_StatePoseController_H
#define traktor_animation_StatePoseController_H

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

class StateGraph;
class StateNode;
class StateContext;

/*! \brief Animation pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS StatePoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	StatePoseController(const resource::Proxy< StateGraph >& stateGraph);

	bool setState(const std::wstring& stateName);

	void setCondition(const std::wstring& condition, bool enabled, bool reset);

	void setTime(float time);

	float getTime() const;

	void setTimeFactor(float timeFactor);

	float getTimeFactor() const;

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
	resource::Proxy< StateGraph > m_stateGraph;
	Ref< StateNode > m_currentState;
	StateContext m_currentStateContext;
	Ref< StateNode > m_nextState;
	StateContext m_nextStateContext;
	float m_blendState;
	float m_blendDuration;
	std::map< std::wstring, std::pair< bool, bool > > m_conditions;
	float m_timeFactor;
};

	}
}

#endif	// traktor_animation_StatePoseController_H
