#ifndef traktor_input_InputDriverPs3_H
#define traktor_input_InputDriverPs3_H

#include "Core/Heap/Ref.h"
#include "Input/InputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDevicePs3;

class T_DLLCLASS InputDriverPs3 : public InputDriver
{
	T_RTTI_CLASS(InputDriverPs3)

public:
	InputDriverPs3(int padCount = 4);

	virtual int getDeviceCount();

	virtual InputDevice* getDevice(int index);

private:
	RefArray< InputDevicePs3 > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverPs3_H
