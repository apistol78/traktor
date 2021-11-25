#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

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

/*! Animation graph transition.
 * \ingroup Animation
 */
class T_DLLCLASS Transition : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum Moment
	{
		TmImmediatly,
		TmEnd
	};

	Transition() = default;

	explicit Transition(StateNode* from, StateNode* to);

	StateNode* from() const;

	StateNode* to() const;

	Moment getMoment() const;

	float getDuration() const;

	const std::wstring& getCondition() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< StateNode > m_from;
	Ref< StateNode > m_to;
	Moment m_moment = TmEnd;
	float m_duration = 0.0f;
	std::wstring m_condition;
};

	}
}

