#ifndef traktor_input_InputValueSource_H
#define traktor_input_InputValueSource_H

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

class IInputDevice;
class InputSystem;
class InputValueSet;
class InputValueSourceData;

class T_DLLCLASS InputValueSource : public Object
{
	T_RTTI_CLASS;

public:
	InputValueSource(const InputValueSourceData* data);
	
	void update(InputSystem* inputSystem, InputValueSet& outValueSet);
	
private:
	Ref< const InputValueSourceData > m_data;
	Ref< IInputDevice > m_device;
	int32_t m_control;
};

	}
}

#endif	// traktor_input_InputValueSource_H
