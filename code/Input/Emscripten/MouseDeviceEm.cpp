#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/Emscripten/MouseDeviceEm.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceEm", MouseDeviceEm, IInputDevice)

MouseDeviceEm::MouseDeviceEm()
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

std::wstring MouseDeviceEm::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDeviceEm::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceEm::isConnected() const
{
	return true;
}

int32_t MouseDeviceEm::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceEm::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceEm::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceEm::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceEm::getControlValue(int32_t control)
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

bool MouseDeviceEm::getControlRange(int32_t control, float& outMin, float& outMax) const
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

bool MouseDeviceEm::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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

bool MouseDeviceEm::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceEm::resetState()
{
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button1 = 0.0f;
	m_button2 = 0.0f;
	m_button3 = 0.0f;
}

void MouseDeviceEm::readState()
{
}

bool MouseDeviceEm::supportRumble() const
{
	return false;
}

void MouseDeviceEm::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceEm::setExclusive(bool exclusive)
{
}

	}
}
