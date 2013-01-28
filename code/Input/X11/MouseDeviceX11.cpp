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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceX11", MouseDeviceX11, IInputDevice)

MouseDeviceX11::MouseDeviceX11(Display* display, Window window, int deviceId)
:	m_display(display)
,	m_window(window)
,	m_deviceId(deviceId)
,	m_connected(false)
,	m_haveCursorPosition(false)
,	m_axisX(0.0f)
,	m_axisY(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
,	m_button1(0.0f)
,	m_button2(0.0f)
,	m_button3(0.0f)
,	m_width(0)
,	m_height(0)
{
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);

	m_width = attr.width;
	m_height = attr.height;

	resetState();
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
		return m_axisX;
	else if (mc.controlType == DtAxisY)
		return m_axisY;
	else if (mc.controlType == DtPositionX)
		return m_positionX;
	else if (mc.controlType == DtPositionY)
		return m_positionY;
	else if (mc.controlType == DtButton1)
		return m_button1;
	else if (mc.controlType == DtButton2)
		return m_button2;
	else if (mc.controlType == DtButton3)
		return m_button3;
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
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button1 = 0.0f;
	m_button2 = 0.0f;
	m_button3 = 0.0f;
	m_haveCursorPosition = false;
}

void MouseDeviceX11::readState()
{
	if (!m_connected)
	{
		/*
		uint8_t mask[2] = { 0, 0 };
		XIEventMask evmask;

		evmask.mask = mask;
		evmask.mask_len = sizeof(mask);
		evmask.deviceid = XIAllDevices;

		XISetMask(mask, XI_Motion);
		XISetMask(mask, XI_ButtonPress);
		XISetMask(mask, XI_ButtonRelease);

		if (XIGrabDevice(
			m_display,
			m_deviceId,
			m_window,
			CurrentTime,
			None,
			GrabModeAsync,
			GrabModeAsync,
			False,
			&mask
		) == GrabSuccess)
			m_connected = true;
		*/

		m_connected = true;
	}

	if (m_connected)
	{
		Window rootWindow, childWindow;
		int rootX = 0, rootY = 0;
		int x = 0, y = 0;
		unsigned int mask = 0;

		XQueryPointer(
			m_display,
			m_window,
			&rootWindow,
			&childWindow,
			&rootX,
			&rootY,
			&x,
			&y,
			&mask
		);

		m_positionX = clamp(float(x), 0.0f, float(m_width));
		m_positionY = clamp(float(y), 0.0f, float(m_height));

		m_button1 = (mask & Button1Mask) ? 1.0f : 0.0f;
		m_button2 = (mask & Button2Mask) ? 1.0f : 0.0f;
		m_button3 = (mask & Button3Mask) ? 1.0f : 0.0f;

/*
		bool exclusive = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);

		if (exclusive)
		{
			GetCursorPos(&m_cursorPosition);
			ScreenToClient(m_hWnd, &m_cursorPosition);

			RECT rc;
			GetClientRect(m_hWnd, &rc);

			POINT cursorCenter;
			cursorCenter.x = (rc.right - rc.left) / 2;
			cursorCenter.y = (rc.bottom - rc.top) / 2;

			m_axisX = float(m_cursorPosition.x - cursorCenter.x);
			m_axisY = float(m_cursorPosition.y - cursorCenter.y);

			m_positionX += m_axisX;
			m_positionY += m_axisY;

			m_positionX = clamp(m_positionX, 0.0f, float(rc.right - rc.left));
			m_positionY = clamp(m_positionY, 0.0f, float(rc.bottom - rc.top));

			ClientToScreen(m_hWnd, &cursorCenter);
			SetCursorPos(cursorCenter.x, cursorCenter.y);
		}
		else
		{
			POINT cursorPosition;
			GetCursorPos(&cursorPosition);

			if (!m_haveCursorPosition)
				m_cursorPosition = cursorPosition;

			m_axisX = float(cursorPosition.x - m_cursorPosition.x);
			m_axisY = float(cursorPosition.y - m_cursorPosition.y);

			m_positionX = float(cursorPosition.x);
			m_positionY = float(cursorPosition.y);

			m_cursorPosition = cursorPosition;
			m_haveCursorPosition = true;
		}

		{
			m_button1 = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f;
			m_button2 = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1.0f : 0.0f;
			m_button3 = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1.0f : 0.0f;
		}
*/
	}
	else
		resetState();
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
}

	}
}
