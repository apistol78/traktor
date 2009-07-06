#ifndef traktor_input_InputDriverXi_H
#define traktor_input_InputDriverXi_H

#if defined(_XBOX)
#include <xtl.h>
#else
#include <windows.h>
#include <xinput.h>
#endif
#include "Core/Heap/Ref.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_XI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDeviceXi;

class T_DLLCLASS InputDriverXi : public IInputDriver
{
	T_RTTI_CLASS(InputDriverXi)

public:
	InputDriverXi(DWORD deviceCount = 4);

	virtual int getDeviceCount();

	virtual IInputDevice* getDevice(int index);

private:
	RefArray< InputDeviceXi > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverXi_H
