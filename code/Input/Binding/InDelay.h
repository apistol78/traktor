#ifndef traktor_input_InDelay_H
#define traktor_input_InDelay_H

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
class T_DLLCLASS InDelay : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InDelay();

	InDelay(IInputNode* source, float delay);
	
	virtual Ref< Instance > createInstance() const T_OVERRIDE T_FINAL;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
private:
	friend class InDelayTraits;

	Ref< IInputNode > m_source;
	float m_delay;
};

	}
}

#endif	// traktor_input_InDelay_H
