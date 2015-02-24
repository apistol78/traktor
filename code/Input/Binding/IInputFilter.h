#ifndef traktor_input_IInputFilter_H
#define traktor_input_IInputFilter_H

#include "Core/Serialization/ISerializable.h"

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
	
class InputValueSet;

/*! \brief Abstract input signal filter.
 * \ingroup Input
 *
 * Filters are used to ensure input values
 * are sane before updating states.
 */
class T_DLLCLASS IInputFilter : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Instance : public IRefCount {};

	virtual Ref< Instance > createInstance() const = 0;

	virtual void evaluate(Instance* instance, InputValueSet& valueSet) const = 0;
};

	}
}

#endif	// traktor_input_IInputFilter_H
