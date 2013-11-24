#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/X11/MouseDeviceX11.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct MouseControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	bool stable;
}
c_mouseControlMap[] =
{
	{ L"Left mouse button", DtButton1, false, true },
	{ L"Right mouse button", DtButton2, false, true },
	{ L"Middle mouse button", DtButton3, false, true },
	{ L"Mouse X axis", DtAxisX, true, true },
	{ L"Mouse Y axis", DtAxisY, true, true },
	{ L"Mouse X axis", DtPositionX, true, false },
	{ L"Mouse Y axis", DtPositionY, true, false }
};

const float c_mouseMargin = 16.0f;
const float c_mouseDeltaScale = 4.0f;
const float c_mouseDeltaLimit = 100.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceX11", MouseDeviceX11, InputDeviceX11)

MouseDeviceX11::MouseDeviceX11(Display* display, Window window, int deviceId)
:	m_display(display)
,	m_window(window)
,	m_deviceId(deviceId)
,	m_connected(true)
,	m_exclusive(false)
,	m_width(0)
,	m_height(0)
{
	uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	XIEventMask evmask;

	// Select motion from user window.
	evmask.mask = mask;
	evmask.mask_len = sizeof(mask);
	evmask.deviceid = m_deviceId;

	XISetMask(mask, XI_Motion);

	XISelectEvents(m_display, m_window, &evmask, 1);

	// Select raw events from default root window.
	evmask.mask = mask;
	evmask.mask_len = sizeof(mask);
	evmask.deviceid = XIAllDevices; //m_deviceId;

	XISetMask(mask, XI_RawMotion);
	XISetMask(mask, XI_RawButtonPress);
	XISetMask(mask, XI_RawButtonRelease);

	XISelectEvents(m_display, DefaultRootWindow(m_display), &evmask, 1);

	resetState();
}

MouseDeviceX11::~MouseDeviceX11()
{
	setExclusive(false);
}

std::wstring MouseDeviceX11::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDeviceX11::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceX11::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceX11::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceX11::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceX11::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceX11::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceX11::getControlValue(int32_t control)
{
	if (!m_connected || control < 0)
		return 0.0f;

	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtAxisX)
		return m_axis[0];
	else if (mc.controlType == DtAxisY)
		return m_axis[1];
	else if (mc.controlType == DtPositionX)
		return m_position[0];
	else if (mc.controlType == DtPositionY)
		return m_position[1];
	else if (mc.controlType == DtButton1)
		return m_button[0];
	else if (mc.controlType == DtButton2)
		return m_button[1];
	else if (mc.controlType == DtButton3)
		return m_button[2];
	else
		return 0.0f;
}

bool MouseDeviceX11::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	if (!m_connected || control < 0)
		return false;

	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtPositionX)
	{
		outMin = 0;
		outMax = float(m_width);
	}
	else if (mc.controlType == DtPositionY)
	{
		outMin = 0;
		outMax = float(m_height);
	}
	else
		return false;

	return true;
}

bool MouseDeviceX11::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	control = -1;
	return false;
}

bool MouseDeviceX11::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceX11::resetState()
{
	for (int i = 0; i < 2; ++i)
	{
		m_raw[i] = 0.0f;
		m_axis[i] = 0.0f;
		m_position[i] = 0.0f;
	}
	m_button[0] = 0.0f;
	m_button[1] = 0.0f;
	m_button[2] = 0.0f;
}

void MouseDeviceX11::readState()
{
	if (!m_connected)
		resetState();

	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);

	m_width = attr.width;
	m_height = attr.height;

	for (int i = 0; i < 2; ++i)
	{
		m_axis[i] = m_raw[i];
		m_raw[i] = 0.0f;
	}

	if (m_exclusive)
	{
		m_position[0] = clamp< float >(m_position[0], c_mouseMargin, m_width - c_mouseMargin);
		m_position[1] = clamp< float >(m_position[1], c_mouseMargin, m_height - c_mouseMargin);

		XWarpPointer(
			m_display,
			None,
			m_window,
			0,
			0,
			0,
			0,
			int(m_position[0]),
			int(m_position[1])
		);
	}
}

bool MouseDeviceX11::supportRumble() const
{
	return false;
}

void MouseDeviceX11::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceX11::setExclusive(bool exclusive)
{
	if (exclusive)
	{
		uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		XIEventMask evmask;

		evmask.mask = mask;
		evmask.mask_len = sizeof(mask);
		evmask.deviceid = XIAllDevices; //m_deviceId;

		XISetMask(mask, XI_RawMotion);
		XISetMask(mask, XI_RawButtonPress);
		XISetMask(mask, XI_RawButtonRelease);

#if !defined(_DEBUG)
		XIGrabDevice(
			m_display,
			m_deviceId,
			DefaultRootWindow(m_display),
			CurrentTime,
			None,
			GrabModeAsync,
			GrabModeAsync,
			False,
			&evmask
		);
#endif

		m_exclusive = true;
	}
	else
	{
#if !defined(_DEBUG)
		XIUngrabDevice(m_display, m_deviceId, CurrentTime);
#endif
		m_exclusive = false;
	}
}

void MouseDeviceX11::consumeEvent(XEvent& evt)
{
	XIEvent* xi = (XIEvent*)evt.xcookie.data;
	switch (xi->evtype)
	{
	case XI_Motion:
		{
			XIDeviceEvent* event = (XIDeviceEvent*)evt.xcookie.data;
			if (event->deviceid != m_deviceId)
				return;

			m_position[0] = float(event->event_x);
			m_position[1] = float(event->event_y);
		}
		break;

	case XI_RawMotion:
		{
			XIRawEvent* event = (XIRawEvent*)evt.xcookie.data;
			if (event->deviceid != m_deviceId)
				return;

			// Use filtered values if available; otherwise use the values directly from the device.
			const double* values = event->valuators.values;
			if (!values)
				values = event->raw_values;

			for (uint32_t i = 0, j = 0; i < event->valuators.mask_len * 8; ++i)
			{
				if (!XIMaskIsSet(event->valuators.mask, i))
					continue;

				m_raw[j++] = clamp(float(values[i] * c_mouseDeltaScale), -c_mouseDeltaLimit, c_mouseDeltaLimit);
				if (j >= 2)
					break;
			}
		}
		break;

	case XI_RawButtonPress:
	case XI_RawButtonRelease:
		{
			XIRawEvent* event = (XIRawEvent*)evt.xcookie.data;
			/*
			if (event->deviceid != m_deviceId)
				return;
			*/
			if (event->detail >= 1 && event->detail <= 3)
				m_button[event->detail - 1] = (xi->evtype == XI_RawButtonPress) ? 1.0f : 0.0f;
		}
		break;

	default:
		break;
	}
}

	}
}
