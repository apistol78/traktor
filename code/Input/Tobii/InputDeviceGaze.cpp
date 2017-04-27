/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Tobii/InputDeviceGaze.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceGaze", InputDeviceGaze, IInputDevice)

InputDeviceGaze::InputDeviceGaze()
:	m_connected(false)
,	m_rangeX(0.0f)
,	m_rangeY(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
{
}

std::wstring InputDeviceGaze::getName() const
{
	return L"Tobii EyeX Gaze";
}

InputCategory InputDeviceGaze::getCategory() const
{
	return CtGaze;
}

bool InputDeviceGaze::isConnected() const
{
	return m_connected;
}

int32_t InputDeviceGaze::getControlCount()
{
	return int32_t(2);
}

std::wstring InputDeviceGaze::getControlName(int32_t control)
{
	switch (control)
	{
	case 0:
		return L"X";

	case 1:
		return L"Y";

	default:
		return L"";
	}
}

bool InputDeviceGaze::isControlAnalogue(int32_t control) const
{
	return true;
}

bool InputDeviceGaze::isControlStable(int32_t control) const
{
	return false;
}

float InputDeviceGaze::getControlValue(int32_t control)
{
	switch (control)
	{
	case 0:
		return m_positionX;

	case 1:
		return m_positionY;

	default:
		return 0.0f;
	}
}

bool InputDeviceGaze::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	switch (control)
	{
	case 0:
		outMin = 0.0f;
		outMax = m_rangeX;
		return true;

	case 1:
		outMin = 0.0f;
		outMax = m_rangeY;
		return true;

	default:
		return 0.0f;
	}
}

bool InputDeviceGaze::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (!analogue)
		return false;

	switch (controlType)
	{
	case DtPositionX:
		control = 0;
		return true;

	case DtPositionY:
		control = 1;
		return true;

	default:
		break;
	}

	return false;
}

bool InputDeviceGaze::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceGaze::resetState()
{
	m_positionX =
	m_positionY = 0.0f;
}

void InputDeviceGaze::readState()
{
}

bool InputDeviceGaze::supportRumble() const
{
	return false;
}

void InputDeviceGaze::setRumble(const InputRumble& rumble)
{
}

void InputDeviceGaze::setExclusive(bool exclusive)
{
}

	}
}
