#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"
#include "Animation/IPoseController.h"
#include "Animation/Pose.h"
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

/*! Animation pose evaluation controller.
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

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

private:
	resource::Proxy< StateGraph > m_stateGraph;
	Ref< StateNode > m_currentState;
	StateContext m_currentStateContext;
	Ref< StateNode > m_nextState;
	StateContext m_nextStateContext;
	Pose m_evaluatePose;
	float m_blendState;
	float m_blendDuration;
	SmallMap< std::wstring, std::pair< bool, bool > > m_conditions;
	float m_timeFactor;
};

	}
}

