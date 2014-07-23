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
	{ L"Mouse Z axis", DtAxisZ, true, true },
	{ L"Mouse X axis", DtPositionX, true, false },
	{ L"Mouse Y axis", DtPositionY, true, false }
};

const float c_mouseDeltaScale = 1.0f;
const float c_mouseDeltaLimit = 200.0f;
const float c_mouseWheelDelta = 1.0f;

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
	XISetMask(mask, XI_ButtonPress);
	XISetMask(mask, XI_ButtonRelease);
	XISelectEvents(m_display, m_window, &evmask, 1);

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
	else if (mc.controlType == DtAxisZ)
		return m_axis[2];
	else if (mc.controlType == DtPositionX)
		return float(m_position[0]);
	else if (mc.controlType == DtPositionY)
		return float(m_position[1]);
	else if (mc.controlType == DtButton1)
		return m_button[0];
	else if (mc.controlType == DtButton2)
		return m_button[2];
	else if (mc.controlType == DtButton3)
		return m_button[1];
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
	m_raw[0] = 0;
	m_raw[1] = 0;
	m_raw[2] = 0;

	m_axis[0] = 0.0f;
	m_axis[1] = 0.0f;
	m_axis[2] = 0.0f;

	m_position[0] = 0;
	m_position[1] = 0;

	m_warped[0] = -1;
	m_warped[1] = -1;

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

	// Update virtual position of pointer.
	if (m_exclusive && m_focus)
	{
		m_position[0] = clamp(m_position[0] + m_raw[0], 0, m_width - 1);
		m_position[1] = clamp(m_position[1] + m_raw[1], 0, m_height - 1);
	}

	// Calculate axis deltas from raw deltas.
	for (int i = 0; i < 3; ++i)
	{
		m_axis[i] = clamp(float(m_raw[i] * c_mouseDeltaScale), -c_mouseDeltaLimit, c_mouseDeltaLimit);
		m_raw[i] = 0;
	}

	if (m_exclusive && m_focus)
	{
		// Warp pointer to center of window.
		m_warped[0] = m_width / 2;
		m_warped[1] = m_height / 2;

		XWarpPointer(
			m_display,
			None,
			m_window,
			0,
			0,
			0,
			0,
			m_warped[0],
			m_warped[1]
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
	if (exclusive && m_focus)
	{
		uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		XIEventMask evmask;

		evmask.mask = mask;
		evmask.mask_len = sizeof(mask);
		evmask.deviceid = XIAllDevices;

		XISetMask(mask, XI_Motion);
		XISetMask(mask, XI_ButtonPress);
		XISetMask(mask, XI_ButtonRelease);

#if !defined(_DEBUG)
		XIGrabDevice(
			m_display,
			m_deviceId,
			m_window,
			CurrentTime,
			None,
			GrabModeAsync,
			GrabModeAsync,
			False,
			&evmask
		);
#endif

	}
	else
	{
#if !defined(_DEBUG)
		XIUngrabDevice(m_display, m_deviceId, CurrentTime);
#endif
	}
	m_exclusive = exclusive;
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

			// Need to check if event's position differ from warped position.
			if (event->event_x != m_warped[0] || event->event_y != m_warped[1])
			{
				if (!m_exclusive || !m_focus)
				{
					int32_t dx = event->event_x - m_position[0];
					int32_t dy = event->event_y - m_position[1];

					m_raw[0] += dx;
					m_raw[1] += dy;
				}
				else
				{
					int32_t cx = m_width / 2;
					int32_t cy = m_height / 2;

					// In exclusive mode the pointer is continiously warped back to center,
					// thus the motion is relative center.
					int32_t dx = event->event_x - cx;
					int32_t dy = event->event_y - cy;

					m_raw[0] += dx;
					m_raw[1] += dy;
				}
			}

			// In exclusive mode the position is accumulated in readState.
			if (!m_exclusive || !m_focus)
			{
				m_position[0] = event->event_x;
				m_position[1] = event->event_y;
			}

			m_warped[0] = -1;
			m_warped[1] = -1;
		}
		break;

	case XI_ButtonPress:
	case XI_ButtonRelease:
		{
			XIDeviceEvent* event = (XIDeviceEvent*)evt.xcookie.data;
			if (event->deviceid != m_deviceId)
				return;

			if (event->detail >= 1 && event->detail <= 3)
				m_button[event->detail - 1] = (xi->evtype == XI_ButtonPress) ? 1.0f : 0.0f;
			else if (xi->evtype == XI_ButtonPress && (event->detail == 4 || event->detail == 5))
				m_raw[2] += (event->detail == 4) ? c_mouseWheelDelta : -c_mouseWheelDelta;
		}
		break;

	default:
		break;
	}
}

void MouseDeviceX11::setFocus(bool focus)
{
	m_focus = focus;
	m_connected = focus;
	setExclusive(m_exclusive);
}

	}
}
