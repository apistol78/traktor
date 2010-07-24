#ifndef traktor_input_InCombine3_H
#define traktor_input_InCombine3_H

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

class T_DLLCLASS InCombine3 : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InCombine3();

	InCombine3(
		IInputNode* source1, float mul1, float add1,
		IInputNode* source2, float mul2, float add2,
		IInputNode* source3, float mul3, float add3
	);

	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_source[3];
	float m_valueMul[3];
	float m_valueAdd[3];
};

	}
}

#endif	// traktor_input_InCombine3_H
