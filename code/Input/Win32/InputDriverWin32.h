#ifndef traktor_input_InputDriverWin32_H
#define traktor_input_InputDriverWin32_H

#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct SystemWindow;

	namespace input
	{

class KeyboardDeviceWin32;
class MouseDeviceWin32;

class T_DLLCLASS InputDriverWin32 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverWin32();

	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	Ref< KeyboardDeviceWin32 > m_keyboardDevice;
	Ref< MouseDeviceWin32 > m_mouseDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverWin32_H
