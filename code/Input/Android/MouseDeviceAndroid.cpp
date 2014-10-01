#include <android/input.h>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/Android/MouseDeviceAndroid.h"

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
	{ L"Mouse button", DtButton1, false, true },
	{ L"Mouse X axis", DtAxisX, true, true },
	{ L"Mouse Y axis", DtAxisY, true, true },
	{ L"Mouse X axis", DtPositionX, true, false },
	{ L"Mouse Y axis", DtPositionY, true, false }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceAndroid", MouseDeviceAndroid, IInputDevice)

MouseDeviceAndroid::MouseDeviceAndroid()
:	m_axisX(0.0f)
,	m_axisY(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
,	m_button(false)
{
	resetState();
}

std::wstring MouseDeviceAndroid::getName() const
{
	return L"Touch Mouse";
}

InputCategory MouseDeviceAndroid::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceAndroid::isConnected() const
{
	return true;
}

int32_t MouseDeviceAndroid::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceAndroid::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceAndroid::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceAndroid::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceAndroid::getControlValue(int32_t control)
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
		return m_button ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool MouseDeviceAndroid::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtPositionX)
	{
		outMin = 0.0f;
		outMax = 1920.0f;
		return true;
	}
	else if (mc.controlType == DtPositionY)
	{
		outMin = 0.0f;
		outMax = 1200.0f;
		return true;
	}
	else
		return false;
}

bool MouseDeviceAndroid::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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

bool MouseDeviceAndroid::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceAndroid::resetState()
{
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button = false;
}

void MouseDeviceAndroid::readState()
{
}

bool MouseDeviceAndroid::supportRumble() const
{
	return false;
}

void MouseDeviceAndroid::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceAndroid::setExclusive(bool exclusive)
{
}

void MouseDeviceAndroid::handleInput(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
		return;

	int32_t action = AMotionEvent_getAction(event);
	uint32_t flags = action & AMOTION_EVENT_ACTION_MASK;

	switch (flags)
	{
	case AMOTION_EVENT_ACTION_DOWN:
		{
			m_button = true;
		}
		break;

	case AMOTION_EVENT_ACTION_UP:
		{
			m_button = false;
		}
		break;
	}

	float positionX = AMotionEvent_getX(event, 0);
	float positionY = AMotionEvent_getY(event, 0);

	m_axisX = positionX - m_positionX;
	m_axisY = positionY - m_positionY;

	m_positionX = positionX;
	m_positionY = positionY;
}

	}
}
