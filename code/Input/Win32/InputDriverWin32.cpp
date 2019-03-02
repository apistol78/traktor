#include <windows.h>
#include "Core/Platform.h"
#include "Input/Win32/InputDriverWin32.h"
#include "Input/Win32/KeyboardDeviceWin32.h"
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverWin32", 0, InputDriverWin32, IInputDriver)

InputDriverWin32::InputDriverWin32()
{
}

bool InputDriverWin32::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	if (inputCategories & CtKeyboard)
	{
		m_keyboardDevice = new KeyboardDeviceWin32(syswin.hWnd);
		m_devices.push_back(m_keyboardDevice);
	}
	if (inputCategories & CtMouse)
	{
		m_mouseDevice = new MouseDeviceWin32(syswin.hWnd);
		m_devices.push_back(m_mouseDevice);
	}
	update();
	return true;
}

int InputDriverWin32::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverWin32::getDevice(int index)
{
	if (index < int(m_devices.size()))
		return m_devices[index];
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverWin32::update()
{
	// Ensure devices are "connected" only when the application
	// is active.

	HWND hWndActive = GetActiveWindow();
	DWORD dwPID = 0; GetWindowThreadProcessId(hWndActive, &dwPID);

	bool connected = bool(GetCurrentProcessId() == dwPID);

	if (m_keyboardDevice)
		m_keyboardDevice->m_connected = connected;

	if (m_mouseDevice)
		m_mouseDevice->m_connected = connected;

	return UrOk;
}

	}
}
