#ifndef traktor_input_InPulse_H
#define traktor_input_InPulse_H

#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InPulse : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InPulse();
	
	virtual InputValue evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_source;
	float m_delay;
	float m_interval;
};

	}
}

#endif	// traktor_input_InPulse_H
