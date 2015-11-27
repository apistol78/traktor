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

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InPulse : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InPulse();

	InPulse(IInputNode* source, float delay, float interval);
	
	virtual Ref< Instance > createInstance() const T_OVERRIDE T_FINAL;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
private:
	friend class InPulseTraits;

	Ref< IInputNode > m_source;
	float m_delay;
	float m_interval;
};

	}
}

#endif	// traktor_input_InPulse_H
