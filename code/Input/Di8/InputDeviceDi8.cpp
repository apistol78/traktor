#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/Di8/InputDeviceDi8.h"
#include "Input/Di8/TypesDi8.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

int getPadAxisValue(DIJOYSTATE* state, const PadAxisInfo& padAxisInfo)
{
	if (padAxisInfo.guidType == GUID_XAxis)
		return state->lX;
	if (padAxisInfo.guidType == GUID_YAxis)
		return state->lY;
	if (padAxisInfo.guidType == GUID_ZAxis)
		return state->lZ;
	if (padAxisInfo.guidType == GUID_RxAxis)
		return state->lRx;
	if (padAxisInfo.guidType == GUID_RyAxis)
		return state->lRy;
	if (padAxisInfo.guidType == GUID_RzAxis)
		return state->lRz;
	if (padAxisInfo.guidType == GUID_Slider)
		return state->rglSlider[padAxisInfo.sliderIndex];
	return 0;
}

float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

bool povInRange(DWORD pov, DWORD mn, DWORD mx)
{
	if (LOWORD(pov) == 0xffff)
		return false;
	else
		return pov >= mn && pov <= mx;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceDi8", InputDeviceDi8, IInputDevice)

InputDeviceDi8::InputDeviceDi8(IDirectInputDevice8* device)
:	m_device(device)
,	m_state(0)
,	m_connected(false)
{
	HRESULT hr;
	
	std::memset(&m_deviceDesc, 0, sizeof(DIDEVICEINSTANCE));
	m_deviceDesc.dwSize = sizeof(DIDEVICEINSTANCE);

	hr = device->GetDeviceInfo(&m_deviceDesc);
	T_ASSERT(SUCCEEDED(hr));

	switch (getCategory())
	{
	case CtMouse:
		device->SetDataFormat(&c_dfDIMouse);
		m_state = new DIMOUSESTATE;
		break;

	case CtKeyboard:
		device->SetDataFormat(&c_dfDIKeyboard);
		m_state = new uint8_t [256];
		break;

	case CtJoystick:
		device->SetDataFormat(&c_dfDIJoystick2);

		collectGamepadInfo(device);
		m_state = new DIJOYSTATE2;
		break;
	}

	hr = device->Acquire();
	m_connected = SUCCEEDED(hr);
}

InputDeviceDi8::~InputDeviceDi8()
{
	delete m_state;
}

std::wstring InputDeviceDi8::getName() const
{
	return m_deviceDesc.tszInstanceName;
}

InputCategory InputDeviceDi8::getCategory() const
{
	switch (m_deviceDesc.dwDevType & 0xff)
	{
	case DI8DEVTYPE_1STPERSON:
	case DI8DEVTYPE_MOUSE:
		return CtMouse;

	case DI8DEVTYPE_KEYBOARD:
		return CtKeyboard;

	case DI8DEVTYPE_JOYSTICK:
	case DI8DEVTYPE_GAMEPAD:
	case DI8DEVTYPE_DRIVING:
	case DI8DEVTYPE_FLIGHT:
		return CtJoystick;

	default:
		return CtUnknown;
	}
}

bool InputDeviceDi8::isConnected() const
{
	return m_connected;
}

int InputDeviceDi8::getControlCount()
{
	switch (getCategory())
	{
	case CtMouse:
		return 0;	// TODO: Fix me

	case CtKeyboard:
		return sizeof_array(c_di8ControlKeys);

	case CtJoystick:
		return 0;	// TODO: Fix me
	}
	return 0;
}

std::wstring InputDeviceDi8::getControlName(int control)
{
	return L"";
}

bool InputDeviceDi8::isControlAnalogue(int control) const
{
	return false;
}

float InputDeviceDi8::getControlValue(int control)
{
	if (!m_connected)
		return 0.0f;

	switch (getCategory())
	{
	case CtMouse:
		break;

	case CtKeyboard:
		{
			DWORD dik = c_di8ControlKeys[control];
			if (dik != 0)
			{
				const uint8_t* state = static_cast< const uint8_t* >(m_state);
				return (state[dik] & 0x80) ? 1.0f : 0.0f;
			}
			else
				return 0;
		}
		break;

	case CtJoystick:
		{
			DIJOYSTATE* state = static_cast< DIJOYSTATE* >(m_state);
			switch (control)
			{
			case DtUp:
				if (
					povInRange(state->rgdwPOV[0], 31500, 36000) ||
					povInRange(state->rgdwPOV[0], 0, 4500)
				)
					return 1.0f;
				else
					return 0.0f;

			case DtDown:
				if (povInRange(state->rgdwPOV[0], 13500, 22500))
					return 1.0f;
				else
					return 0.0f;

			case DtLeft:
				if (povInRange(state->rgdwPOV[0], 22500, 31500))
					return 1.0f;
				else
					return 0.0f;

			case DtRight:
				if (povInRange(state->rgdwPOV[0], 4500, 13500))
					return 1.0f;
				else
					return 0.0f;

			case DtThumbLeftX:
				{
					if (m_padAxisInfo.size() < 4)
						return 0;
					int v = getPadAxisValue(state, m_padAxisInfo[0]);
					return adjustDeadZone((v - 32767.0f) / 32767.0f);
				}

			case DtThumbLeftY:
				{
					if (m_padAxisInfo.size() < 4)
						return 0;
					int v = getPadAxisValue(state, m_padAxisInfo[1]);
					return adjustDeadZone(-(v - 32767.0f) / 32767.0f);
				}

			case DtThumbRightX:
				{
					if (m_padAxisInfo.size() < 4)
						return 0;
					int v = getPadAxisValue(state, m_padAxisInfo[2]);
					return adjustDeadZone((v - 32767.0f) / 32767.0f);
				}

			case DtThumbRightY:
				{
					if (m_padAxisInfo.size() < 4)
						return 0;
					int v = getPadAxisValue(state, m_padAxisInfo[3]);
					return adjustDeadZone((v - 32767.0f) / 32767.0f);
				}

			case DtButton1:
				return ((state->rgbButtons[1] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtButton2:
				return ((state->rgbButtons[2] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtButton3:
				return ((state->rgbButtons[0] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtButton4:
				return ((state->rgbButtons[3] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtSelect:
				return ((state->rgbButtons[9] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtCancel:
				return ((state->rgbButtons[8] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtTriggerLeft:
				return ((state->rgbButtons[6] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtTriggerRight:
				return ((state->rgbButtons[7] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtShoulderLeft:
				return ((state->rgbButtons[4] & 0x80) != 0) ? 1.0f : 0.0f;

			case DtShoulderRight:
				return ((state->rgbButtons[5] & 0x80) != 0) ? 1.0f : 0.0f;
			}
		}
		break;
	}

	return 0.0f;
}

bool InputDeviceDi8::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	control = controlType;
	return true;
}

void InputDeviceDi8::resetState()
{
}

void InputDeviceDi8::readState()
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
	if (FAILED(hr))  
	{
		hr = m_device->Acquire();
		m_connected = SUCCEEDED(hr);
		if (!m_connected)
			return;
	}

	switch (getCategory())
	{
	case CtMouse:
		hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), m_state);
		break;

	case CtKeyboard:
		hr = m_device->GetDeviceState(256, m_state);
		break;

	case CtJoystick:
		hr = m_device->GetDeviceState(sizeof(DIJOYSTATE2), m_state);
		break;
	}

	m_connected = SUCCEEDED(hr);	
}

bool InputDeviceDi8::supportRumble() const
{
	return false;
}

void InputDeviceDi8::setRumble(const InputRumble& rumble)
{
}

void InputDeviceDi8::collectGamepadInfo(IDirectInputDevice8* device)
{
	DIDEVCAPS capabilities;
	capabilities.dwSize = sizeof(DIDEVCAPS);
	device->GetCapabilities(&capabilities);
	int axesCount = capabilities.dwAxes;
	m_padAxisInfo.resize(axesCount);

	DWORD ids[] =
	{
		DIJOFS_X,
		DIJOFS_Y,
		DIJOFS_Z,
		DIJOFS_RX,
		DIJOFS_RY,
		DIJOFS_RZ,
		DIJOFS_SLIDER(0),
		DIJOFS_SLIDER(1)
	};

	int axisIndex = 0;
	int sliderIndex = 0;
	for (int i = 0; i < sizeof_array(ids); i++)
	{
		DIPROPRANGE range; 
		range.diph.dwSize       = sizeof(DIPROPRANGE); 
		range.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		range.diph.dwHow        = DIPH_BYOFFSET; 
		range.diph.dwObj        = ids[i];
		HRESULT r;
		r = device->GetProperty(DIPROP_RANGE, &range.diph);
		if (r != S_OK)
			continue;
		DIPROPDWORD deadzone;
		deadzone.diph.dwSize       = sizeof(DIPROPDWORD); 
		deadzone.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		deadzone.diph.dwHow        = DIPH_BYOFFSET; 
		deadzone.diph.dwObj        = ids[i];
		r = device->GetProperty(DIPROP_DEADZONE, &deadzone.diph);

		DIPROPDWORD saturation;
		saturation.diph.dwSize       = sizeof(DIPROPDWORD); 
		saturation.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		saturation.diph.dwHow        = DIPH_BYOFFSET; 
		saturation.diph.dwObj        = ids[i];
		r = device->GetProperty(DIPROP_SATURATION, &saturation.diph);

		DIDEVICEOBJECTINSTANCE didoi;
		didoi.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		r = device ->GetObjectInfo(&didoi, ids[i], DIPH_BYOFFSET);

		m_padAxisInfo[axisIndex].name = std::wstring(didoi.tszName);
		m_padAxisInfo[axisIndex].guidType = didoi.guidType;
		m_padAxisInfo[axisIndex].rangeMax = range.lMax;
		m_padAxisInfo[axisIndex].rangeMin = range.lMin;
		m_padAxisInfo[axisIndex].deadzone = deadzone.dwData;
		m_padAxisInfo[axisIndex].saturation = saturation.dwData;
		m_padAxisInfo[axisIndex].sliderIndex = sliderIndex;
		if (didoi.guidType == GUID_Slider)
			sliderIndex++;
		axisIndex++;
	}
}
	}
}
