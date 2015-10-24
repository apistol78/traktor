#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/KeyboardDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceDi8", KeyboardDeviceDi8, IInputDevice)

KeyboardDeviceDi8::KeyboardDeviceDi8(HWND hWnd, IDirectInputDevice8* device, const DIDEVICEINSTANCE* deviceInstance)
:	m_hWnd(hWnd)
,	m_pWndProc(0)
,	m_device(device)
,	m_connected(false)
{
	// Subclass window to get access to window events.
	m_pWndProc = (WNDPROC)GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)&KeyboardDeviceDi8::wndProc);
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	// Setup data format.
	m_device->SetDataFormat(&c_dfDIKeyboard);
	m_name = tstows(deviceInstance->tszInstanceName);

	// Initially reset state.
	resetState();

	// Try to acquire device.
	HRESULT hr = device->Acquire();
	m_connected = SUCCEEDED(hr);
}

void KeyboardDeviceDi8::destroy()
{
	if (m_pWndProc)
	{
		SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pWndProc);
		m_pWndProc = 0;
	}
}

std::wstring KeyboardDeviceDi8::getName() const
{
	return m_name;
}

InputCategory KeyboardDeviceDi8::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceDi8::isConnected() const
{
	return m_connected;
}

int32_t KeyboardDeviceDi8::getControlCount()
{
	return sizeof_array(c_di8ControlKeys);
}

std::wstring KeyboardDeviceDi8::getControlName(int32_t control)
{
	HRESULT hr;

	// Query key name through object info method.
	{
		DIDEVICEOBJECTINSTANCE didoi;
		std::memset(&didoi, 0, sizeof(didoi));
		didoi.dwSize = sizeof(didoi);
		didoi.guidType = GUID_Key;

		hr = m_device->GetObjectInfo(&didoi, c_di8ControlKeys[control], DIPH_BYOFFSET);
		if (SUCCEEDED(hr))
			return tstows(didoi.tszName);
	}

	// Query key name as a property.
	{
		DIPROPSTRING dips;
		std::memset(&dips, 0, sizeof(dips));
		dips.diph.dwSize = sizeof(DIPROPSTRING); 
		dips.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		dips.diph.dwObj = c_di8ControlKeys[control]; 
		dips.diph.dwHow = DIPH_BYOFFSET; 
		
		hr = m_device->GetProperty(DIPROP_KEYNAME, (LPDIPROPHEADER)&dips);
		if (SUCCEEDED(hr))
			return dips.wsz;
	}

	// Fallback to query scan code; then use regular Win32 api to get key name.
	{
		DIPROPDWORD dipdw;
		std::memset(&dipdw, 0, sizeof(dipdw));
		dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		dipdw.diph.dwObj = c_di8ControlKeys[control]; 
		dipdw.diph.dwHow = DIPH_BYOFFSET; 

		hr = m_device->GetProperty(DIPROP_SCANCODE, (LPDIPROPHEADER)&dipdw);
		if (SUCCEEDED(hr))
		{
			UINT scanCode = dipdw.dwData;

			switch (c_di8ControlKeys[control])
			{
			case DIK_LEFT: case DIK_UP: case DIK_RIGHT: case DIK_DOWN:
			case DIK_PRIOR: case DIK_NEXT: case DIK_END: case DIK_HOME:
			case DIK_INSERT: case DIK_DELETE: case DIK_DIVIDE: case DIK_NUMLOCK:
				scanCode |= 0x100;
				break;
			}

			TCHAR keyName[50];
			if (GetKeyNameText(scanCode << 16, keyName, sizeof_array(keyName)) != 0)
				return tstows(keyName);
			else
				return L"";
		}
	}

	// Unable to resolve key name.
	return L"";
}

bool KeyboardDeviceDi8::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceDi8::isControlStable(int32_t control) const
{
	return false;
}

float KeyboardDeviceDi8::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	DWORD dik = c_di8ControlKeys[control];
	if (dik == 0)
		return 0.0f;

	return (m_state[dik] & 0x80) ? 1.0f : 0.0f;
}

bool KeyboardDeviceDi8::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceDi8::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue)
		return false;

	control = controlType;
	return c_di8ControlKeys[control] != 0;
}

bool KeyboardDeviceDi8::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();

	return true;
}

void KeyboardDeviceDi8::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
	m_keyEvents.clear();
}

void KeyboardDeviceDi8::readState()
{
	HRESULT hr;

	if (!m_connected)
	{
		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;
	}

	hr = m_device->Poll();
	while (FAILED(hr))  
	{
		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;

		hr = m_device->Poll();
	}

	hr = m_device->GetDeviceState(256, m_state);

	m_connected = SUCCEEDED(hr);	
}

bool KeyboardDeviceDi8::supportRumble() const
{
	return false;
}

void KeyboardDeviceDi8::setRumble(const InputRumble& rumble)
{
}

void KeyboardDeviceDi8::setExclusive(bool exclusive)
{
	// Ensure device is un-aquired, cannot change cooperative level if acquired.
	m_device->Unacquire();
	m_connected = false;
}

LRESULT WINAPI KeyboardDeviceDi8::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KeyboardDeviceDi8* this_ = reinterpret_cast< KeyboardDeviceDi8* >(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	T_ASSERT (this_);

	if (uMsg == WM_CHAR)
	{
		if (wParam != '\r')
		{
			KeyEvent ke;
			ke.type = KtCharacter;
			ke.character = (wchar_t)wParam;
			this_->m_keyEvents.push_back(ke);
		}
	}
	else if (uMsg == WM_KEYDOWN)
	{
		uint32_t keyCode = translateFromVk(uint32_t(wParam));
		if (keyCode != 0)
		{
			KeyEvent ke;
			ke.type = KtDown;
			ke.keyCode = keyCode;
			this_->m_keyEvents.push_back(ke);
		}
	}
	else if (uMsg == WM_KEYUP)
	{
		uint32_t keyCode = translateFromVk(uint32_t(wParam));
		if (keyCode != 0)
		{
			KeyEvent ke;
			ke.type = KtUp;
			ke.keyCode = keyCode;
			this_->m_keyEvents.push_back(ke);
		}
	}

	return CallWindowProc(this_->m_pWndProc, hWnd, uMsg, wParam, lParam);
}

	}
}
