#ifndef traktor_input_InRemapAxis_H
#define traktor_input_InRemapAxis_H

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

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InRemapAxis : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InRemapAxis();

	InRemapAxis(IInputNode* source, float limitMin, float limitMax, float outputMin, float outputMid, float outputMax);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;

	virtual void serialize(ISerializer& s);

private:
	friend class InRemapAxisTraits;

	Ref< IInputNode > m_source;
	float m_limit[2];
	float m_output[3];
};

	}
}

#endif	// traktor_input_InRemapAxis_H
