#include <wbemidl.h>
#include <oleauto.h>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Input/Di8/InputDriverDi8.h"
#include "Input/Di8/JoystickDeviceDi8.h"
#include "Input/Di8/KeyboardDeviceDi8.h"
#include "Input/Di8/MouseDeviceDi8.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

#define SAFE_RELEASE(instance) \
	if (instance) \
	{ \
		instance->Release(); \
		instance = 0; \
	}

/*! \brief Check if device is a XInput device.
 *
 * Enum each PNP device using WMI and check each device ID to see if it contains 
 * "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
 * Unfortunately this information can not be found by just using DirectInput.
 *
 * \note
 * http://msdn.microsoft.com/en-us/library/ee417014(VS.85).aspx
 */
BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput)
{
	IWbemLocator* pIWbemLocator = NULL;
	IEnumWbemClassObject* pEnumDevices = NULL;
	IWbemClassObject* pDevices[20] = {0};
	IWbemServices* pIWbemServices = NULL;
	BSTR bstrNamespace = NULL;
	BSTR bstrDeviceID = NULL;
	BSTR bstrClassName = NULL;
	DWORD uReturned = 0;
	bool bIsXinputDevice = false;
	UINT iDevice = 0;
	VARIANT var;
	HRESULT hr;

	// CoInit if needed
	hr = CoInitialize(NULL);
	bool bCleanupCOM = SUCCEEDED(hr);

	// Create WMI
	hr = CoCreateInstance(
		__uuidof(WbemLocator),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWbemLocator),
		(LPVOID*)&pIWbemLocator
	);
	if (FAILED(hr) || pIWbemLocator == NULL)
		goto LCleanup;

	bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
	if (bstrNamespace == NULL)
		goto LCleanup;        
	bstrClassName = SysAllocString(L"Win32_PNPEntity");
	if (bstrClassName == NULL)
		goto LCleanup;        
	bstrDeviceID = SysAllocString(L"DeviceID");
	if (bstrDeviceID == NULL)
		goto LCleanup;        

	// Connect to WMI 
	hr = pIWbemLocator->ConnectServer(
		bstrNamespace,
		NULL,
		NULL,
		0L, 
		0L,
		NULL,
		NULL,
		&pIWbemServices
	);
	if (FAILED(hr) || pIWbemServices == NULL)
		goto LCleanup;

	// Switch security level to IMPERSONATE. 
	CoSetProxyBlanket(
		pIWbemServices,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL, 
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
	);                    

	hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
	if (FAILED(hr) || pEnumDevices == NULL)
		goto LCleanup;

	// Loop over all devices; no more than 2000 devices.
	for (uint32_t i = 0; i < 100; ++i)
	{
		// Get 20 at a time; timeout in one second to prevent stupid long enumerations.
		hr = pEnumDevices->Next(1000, 20, pDevices, &uReturned);
		if (FAILED(hr))
			goto LCleanup;
		if (uReturned == 0)
			break;

		for (iDevice = 0; iDevice < uReturned; iDevice++)
		{
			// For each device, get its device ID
			hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
			if (SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL)
			{
				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
				// This information can not be found from DirectInput 
				if (wcsstr(var.bstrVal, L"IG_"))
				{
					// If it does, then get the VID/PID from var.bstrVal
					DWORD dwPid = 0, dwVid = 0;
					WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
					if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
						dwVid = 0;
					WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
					if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
						dwPid = 0;

					// Compare the VID/PID to the DInput device
					DWORD dwVidPid = MAKELONG(dwVid, dwPid);
					if (dwVidPid == pGuidProductFromDirectInput->Data1)
					{
						bIsXinputDevice = true;
						goto LCleanup;
					}
				}
			}   
			SAFE_RELEASE(pDevices[iDevice]);
		}
	}

LCleanup:
	if (bstrNamespace)
		SysFreeString(bstrNamespace);
	if (bstrDeviceID)
		SysFreeString(bstrDeviceID);
	if (bstrClassName)
		SysFreeString(bstrClassName);
	for (iDevice = 0; iDevice < 20; iDevice++)
		SAFE_RELEASE(pDevices[iDevice]);
	SAFE_RELEASE(pEnumDevices);
	SAFE_RELEASE(pIWbemLocator);
	SAFE_RELEASE(pIWbemServices);

	if (bCleanupCOM)
		CoUninitialize();

	return bIsXinputDevice;
}

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
		T_DEBUG(L"Using \"" << tstows(text) << L"\" as device window");
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverDi8", 0, InputDriverDi8, IInputDriver)

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

void InputDriverDi8::destroy()
{
	m_directInput.release();
}

bool InputDriverDi8::create(const SystemWindow& systemWindow, uint32_t inputCategories)
{
	HRESULT hr;

	if (!(m_hWnd = systemWindow.hWnd))
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
	InputCategory inputCategory;
	ComRef< IDirectInputDevice8 > device;
	HRESULT hr;

	switch (deviceInstance->dwDevType & 0xff)
	{
	case DI8DEVTYPE_1STPERSON:
	case DI8DEVTYPE_MOUSE:
		inputCategory = CtMouse;
		break;

	case DI8DEVTYPE_KEYBOARD:
		inputCategory = CtKeyboard;
		break;

	case DI8DEVTYPE_JOYSTICK:
	case DI8DEVTYPE_GAMEPAD:
		{
			inputCategory = CtJoystick;
			if (IsXInputDevice(&deviceInstance->guidProduct))
				return true;
		}
		break;

	default:
		return true;
	}

	if ((inputCategory & m_inputCategories) == 0)
		return true;

	log::info << L"Found DI8 device;" << Endl <<
		L"\tInstance name \"" << deviceInstance->tszInstanceName << L"\"" << Endl <<
		L"\tProduct name \"" << deviceInstance->tszProductName << L"\"" << Endl;

	hr = m_directInput->CreateDevice(deviceInstance->guidInstance, &device.getAssign(), NULL);
	if (FAILED(hr)) 
		return false;

	hr = device->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
		return false;

	Ref< input::IInputDevice > inputDevice;
	switch (inputCategory)
	{
	case CtMouse:
		inputDevice = new MouseDeviceDi8(m_hWnd, device.get(), deviceInstance);
		break;
	case CtKeyboard:
		inputDevice = new KeyboardDeviceDi8(m_hWnd, device.get(), deviceInstance);
		break;
	case CtJoystick:
		inputDevice = new JoystickDeviceDi8(m_hWnd, device.get(), deviceInstance);
		break;
	default:
		T_FATAL_ERROR;
	}

	m_devices.push_back(inputDevice);
	return true;
}

	}
}
