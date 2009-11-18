#ifndef traktor_animation_Transition_H
#define traktor_animation_Transition_H

#include "Core/Serialization/ISerializable.h"

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

class State;

/*! \brief Animation graph transition.
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

	Transition();

	Transition(State* from, State* to);

	Ref< State > from() const;

	Ref< State > to() const;

	Moment getMoment() const;

	float getDuration() const;

	const std::wstring& getCondition() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< State > m_from;
	Ref< State > m_to;
	Moment m_moment;
	float m_duration;
	std::wstring m_condition;
};

	}
}

#endif	// traktor_animation_Transition_H
