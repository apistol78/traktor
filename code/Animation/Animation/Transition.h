/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_Transition_H
#define traktor_animation_Transition_H

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

	Transition(StateNode* from, StateNode* to);

	StateNode* from() const;

	StateNode* to() const;

	Moment getMoment() const;

	float getDuration() const;

	const std::wstring& getCondition() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< StateNode > m_from;
	Ref< StateNode > m_to;
	Moment m_moment;
	float m_duration;
	std::wstring m_condition;
};

	}
}

#endif	// traktor_animation_Transition_H
