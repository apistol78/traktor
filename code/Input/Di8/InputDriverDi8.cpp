#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/InputDriverDi8.h"
#include "Input/Di8/InputDeviceDi8.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

BOOL CALLBACK enumWindowProc(HWND hWnd, LPARAM lParam)
{
	HWND* outHwnd = (HWND*)lParam;

	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);

	if (IsWindowVisible(hWnd) && GetCurrentProcessId() == dwProcessId)
	{
#if defined(_DEBUG)
		TCHAR text[256];
		GetWindowText(hWnd, text, sizeof_array(text));
		log::debug << L"Using \"" << tstows(text) << L"\" as device window" << Endl;
#endif
		*outHwnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

HWND getMyProcessWindow()
{
	HWND hWnd = NULL;
	EnumWindows(enumWindowProc, (LPARAM)&hWnd);
	return hWnd;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverDi8", InputDriverDi8, IInputDriver)

InputDriverDi8::InputDriverDi8()
:	m_directInput(0)
,	m_hWnd(NULL)
,	m_inputCategories(0)
{
}

InputDriverDi8::~InputDriverDi8()
{
	destroy();
}

bool InputDriverDi8::create(void* nativeWindowHandle, uint32_t inputCategories)
{
	HRESULT hr;

	if (!(m_hWnd = (HWND)nativeWindowHandle))
	{
		if (!(m_hWnd = getMyProcessWindow()))
		{
			log::error << L"Unable to create input driver; no window handle" << Endl;
			return false;
		}
	}

	hr = DirectInput8Create((HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput.getAssign(), NULL); 
	if (FAILED(hr))
	{
		log::error << L"Unable to create input driver; DirectInput8Create failed, hr = " << int32_t(hr) << Endl;
		return false;
	}

	m_inputCategories = inputCategories;

	hr = m_directInput->EnumDevices(DI8DEVCLASS_ALL, enumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr))
	{
		log::error << L"Unable to create input driver; enumerate devices failed" << Endl;
		return false;
	}

	return true;
}

void InputDriverDi8::destroy()
{
	m_directInput.release();
}

int InputDriverDi8::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverDi8::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverDi8::update()
{
	return UrOk;
}

BOOL CALLBACK InputDriverDi8::enumDevicesCallback(const DIDEVICEINSTANCE* deviceInstance, VOID* context)
{
	InputDriverDi8* driver = static_cast< InputDriverDi8* >(context);
	driver->addDevice(deviceInstance);
	return DIENUM_CONTINUE;
}

bool InputDriverDi8::addDevice(const DIDEVICEINSTANCE* deviceInstance)
{
	ComRef< IDirectInputDevice8 > device;
	HRESULT hr;

	hr = m_directInput->CreateDevice(deviceInstance->guidInstance, &device.getAssign(), NULL);
	if (FAILED(hr)) 
		return false;

	hr = device->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
		return false;

	Ref< input::InputDeviceDi8 > inputDevice = new input::InputDeviceDi8(device.get());
	if ((inputDevice->getCategory() & m_inputCategories) != 0)
		m_devices.push_back(inputDevice);

	return true;
}

	}
}
