#ifndef traktor_input_InputDriverXi_H
#define traktor_input_InputDriverXi_H

#if defined(_XBOX)
#	include <xtl.h>
#else
#	include <windows.h>
#	include <xinput.h>
#endif
#include "Core/RefArray.h"
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
	T_RTTI_CLASS;

public:
	InputDriverXi(DWORD deviceCount = 4);

	virtual bool create(const SystemWindow& systemWindow, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	RefArray< InputDeviceXi > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverXi_H
