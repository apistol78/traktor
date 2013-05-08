#ifndef traktor_input_InConst_H
#define traktor_input_InConst_H

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
class T_DLLCLASS InConst : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InConst();
	
	InConst(float value);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;	

	virtual void serialize(ISerializer& s);
	
private:
	float m_value;
};

	}
}

#endif	// traktor_input_InConst_H
