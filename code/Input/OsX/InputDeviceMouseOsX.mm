#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceMouseOsX.h"
#include "Input/OsX/InputUtilities.h"

namespace traktor
{
	namespace input
	{
		namespace
		{
		
struct MouseControlMap
{
	InputDefaultControlType control;
	int32_t index;
	bool analogue;
	bool stable;
	const wchar_t* name;
}
c_mouseControlMap[] =
{
	{ DtAxisX, -1, true, false, L"Axis X" },
	{ DtAxisY, -2, true, false, L"Axis Y" },
	{ DtAxisZ, -3, true, false, L"Wheel" },
	{ DtButton1, 0, false, true, L"Button 1" },
	{ DtButton2, 1, false, true, L"Button 2" },
	{ DtButton3, 2, false, true, L"Button 3" },
	{ DtButton4, 3, false, true, L"Button 4" },
	{ DtPositionX, -4, true, false, L"Axis X" },
	{ DtPositionY, -5, true, false, L"Axis Y" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceMouseOsX", InputDeviceMouseOsX, IInputDevice)

InputDeviceMouseOsX::InputDeviceMouseOsX()
:	m_exclusive(false)
,	m_lastMouseValid(false)
,	m_scrollAccum(0.0f)
{
	std::memset(m_timeStamps, 0, sizeof(m_timeStamps));
	resetState();
}

std::wstring InputDeviceMouseOsX::getName() const
{
	return L"Mouse";
}

InputCategory InputDeviceMouseOsX::getCategory() const
{
	return CtMouse;
}

bool InputDeviceMouseOsX::isConnected() const
{
	return true;
}

int32_t InputDeviceMouseOsX::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring InputDeviceMouseOsX::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool InputDeviceMouseOsX::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool InputDeviceMouseOsX::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float InputDeviceMouseOsX::getControlValue(int32_t control)
{
	int32_t index = c_mouseControlMap[control].index;
	if (index == -1)
		return m_axis[0];
	else if (index == -2)
		return m_axis[1];
	else if (index == -3)
		return m_axis[2];
    else if (index == -4)
        return m_axis[3];
    else if (index == -5)
        return m_axis[4];
	else if (index >= 0 && index < sizeof_array(m_button))
		return m_button[index] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceMouseOsX::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	int32_t index = c_mouseControlMap[control].index;
	NSSize range;

	if (!getMouseRange(range))
		return false;
	
	if (index == -4)
	{
		outMin = 0.0f;
		outMax = range.width;
	}
	else if (index == -5)
	{
		outMin = 0.0f;
		outMax = range.height;
	}
	else
		return false;
		
	return true;
}

bool InputDeviceMouseOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.control == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool InputDeviceMouseOsX::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceMouseOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
	std::memset(m_axis, 0, sizeof(m_axis));
}

void InputDeviceMouseOsX::readState()
{
	resetState();

	bool mouseValid = isInputAllowed();

	// Capture mouse if exclusive mode.
	if (CGAssociateMouseAndMouseCursorPosition(!m_exclusive) != kCGErrorSuccess)
		return;

	NSPoint centerPosition;
	if (!getMouseCenterPosition(centerPosition))
		return;

	NSPoint mousePositionGlobal, mousePositionLocal;
	if (!getMousePosition(mousePositionGlobal, mousePositionLocal))
		return;

	int32_t deltaX = 0, deltaY = 0;
	CGGetLastMouseDelta(&deltaX, &deltaY);
	m_axis[0] = float(deltaX);
	m_axis[1] = float(deltaY);
	m_axis[2] = m_scrollAccum; m_scrollAccum = 0.0f;

	uint32_t buttons = [NSEvent pressedMouseButtons];
	for (uint32_t i = 0; i < 4; ++i)
		m_button[i] = bool((buttons & (1 << i)) != 0);

	m_axis[3] = int32_t(mousePositionLocal.x);
	m_axis[4] = int32_t(mousePositionLocal.y);

	// As long as user keps mouse button pressed we cannot
	// leave invalid state.
	if (mouseValid && !m_lastMouseValid)
	{
		if (m_button[0] || m_button[1] || m_button[2])
			mouseValid = false;
	}
	
	// Discard all inputs if mouse not in a valid
	// region.
	if (!mouseValid)
		resetState();
	
	//m_lastMousePosition = mousePositionGlobal;
	m_lastMouseValid = mouseValid;
}

bool InputDeviceMouseOsX::supportRumble() const
{
	return false;
}

void InputDeviceMouseOsX::setRumble(const InputRumble& rumble)
{
}

void InputDeviceMouseOsX::setExclusive(bool exclusive)
{
	m_exclusive = exclusive;
}

void InputDeviceMouseOsX::consumeEvent(NSEvent* event)
{
	if ([event type] == NSScrollWheel)
		m_scrollAccum += [event scrollingDeltaY] / 120.0f;
}

	}
}
