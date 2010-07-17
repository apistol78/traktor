#ifndef traktor_input_InCombine_H
#define traktor_input_InCombine_H

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

class T_DLLCLASS InCombine : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	virtual InputValue evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_source[2];
	float m_valueMul[2];
	float m_valueAdd[2];
};

	}
}

#endif	// traktor_input_InCombine_H
