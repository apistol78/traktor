#ifndef traktor_input_InThreshold_H
#define traktor_input_InThreshold_H

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
class T_DLLCLASS InThreshold : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InThreshold();

	InThreshold(IInputNode* source, float duration);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;

	virtual void serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_source;
	float m_duration;
};

	}
}

#endif	// traktor_input_InThreshold_H
