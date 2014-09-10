#include <ppapi/cpp/input_event.h>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/PNaCl/MouseDevicePNaCl.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDevicePNaCl", MouseDevicePNaCl, IInputDevice)

MouseDevicePNaCl::MouseDevicePNaCl()
:	m_axisX(0.0f)
,	m_axisY(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
,	m_button1(0.0f)
,	m_button2(0.0f)
,	m_button3(0.0f)
{
	resetState();
}

std::wstring MouseDevicePNaCl::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDevicePNaCl::getCategory() const
{
	return CtMouse;
}

bool MouseDevicePNaCl::isConnected() const
{
	return true;
}

int32_t MouseDevicePNaCl::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDevicePNaCl::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDevicePNaCl::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDevicePNaCl::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDevicePNaCl::getControlValue(int32_t control)
{
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

bool MouseDevicePNaCl::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtPositionX)
	{
		outMin = 0.0f;
		outMax = 640.0f;
		return true;
	}
	else if (mc.controlType == DtPositionY)
	{
		outMin = 0.0f;
		outMax = 480.0f;
		return true;
	}
	else
		return false;
}

bool MouseDevicePNaCl::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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
	return false;
}

bool MouseDevicePNaCl::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDevicePNaCl::resetState()
{
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button1 = 0.0f;
	m_button2 = 0.0f;
	m_button3 = 0.0f;
}

void MouseDevicePNaCl::readState()
{
}

bool MouseDevicePNaCl::supportRumble() const
{
	return false;
}

void MouseDevicePNaCl::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDevicePNaCl::setExclusive(bool exclusive)
{
}

void MouseDevicePNaCl::consumeEvent(const pp::InputEvent& event)
{
	if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN)
	{
		pp::MouseInputEvent mouseEvent(event);
		switch (mouseEvent.GetButton())
		{
		case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
			m_button1 = 1.0f;
			break;
		case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
			m_button3 = 1.0f;
			break;
		case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
			m_button2 = 1.0f;
			break;
		default:
			break;
		}
	}
	else if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP)
	{
		pp::MouseInputEvent mouseEvent(event);
		switch (mouseEvent.GetButton())
		{
		case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
			m_button1 = 0.0f;
			break;
		case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
			m_button3 = 0.0f;
			break;
		case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
			m_button2 = 0.0f;
			break;
		default:
			break;
		}
	}
	else if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE)
	{
		pp::MouseInputEvent mouseEvent(event);
		
		pp::Point position = mouseEvent.GetPosition();
		pp::Point movement = mouseEvent.GetMovement();

		m_positionX = position.x();
		m_positionY = position.y();

		m_axisX = movement.x();
		m_axisY = movement.y();
	}
}

	}
}
