#ifndef traktor_animation_StateGraph_H
#define traktor_animation_StateGraph_H

#include "Core/RefArray.h"
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
class Transition;

/*! \brief Animation state graph.
 * \ingroup Animation
 */
class T_DLLCLASS StateGraph : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addState(State* state);

	void removeState(State* state);

	const RefArray< State >& getStates() const;

	void addTransition(Transition* transition);

	void removeTransition(Transition* transition);

	const RefArray< Transition >& getTransitions() const;

	void setRootState(State* rootState);

	Ref< State > getRootState() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< State > m_states;
	RefArray< Transition > m_transitions;
	Ref< State > m_rootState;
};

	}
}

#endif	// traktor_animation_StateGraph_H
