#ifndef traktor_input_IInputSource_H
#define traktor_input_IInputSource_H

#include "Core/Object.h"

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

class InputSystem;
class InputValueSet;

class T_DLLCLASS IInputSource : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const = 0;
	
	virtual void prepare(float T, float dT) = 0;

	virtual float read(float T, float dT) = 0;
};

	}
}

#endif	// traktor_input_IInputSource_H
