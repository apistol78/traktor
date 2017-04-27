/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <android/input.h>
#include <android/native_window.h>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/Android/TouchDeviceAndroid.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct TouchControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
}
c_touchControlMap[] =
{
	{ L"Finger 1", DtButton1, false },
	{ L"Finger 2", DtButton2, false },
	{ L"Finger 3", DtButton3, false },
	{ L"Finger 4", DtButton4, false },
	{ L"Touch X axis 1", DtPositionX, true },
	{ L"Touch Y axis 1", DtPositionY, true },
	{ L"Touch X axis 2", DtPositionX2, true },
	{ L"Touch Y axis 2", DtPositionY2, true },
	{ L"Touch X axis 3", DtPositionX3, true },
	{ L"Touch Y axis 3", DtPositionY3, true },
	{ L"Touch X axis 4", DtPositionX4, true },
	{ L"Touch Y axis 4", DtPositionY4, true }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.TouchDeviceAndroid", TouchDeviceAndroid, IInputDevice)

TouchDeviceAndroid::TouchDeviceAndroid(const SystemWindow& syswin)
:	m_width(0)
,	m_height(0)
{
	m_width = ANativeWindow_getWidth(*syswin.window);
	m_height = ANativeWindow_getHeight(*syswin.window);
	resetState();
}

std::wstring TouchDeviceAndroid::getName() const
{
	return L"Touch";
}

InputCategory TouchDeviceAndroid::getCategory() const
{
	return CtTouch;
}

bool TouchDeviceAndroid::isConnected() const
{
	return true;
}

int32_t TouchDeviceAndroid::getControlCount()
{
	return sizeof_array(c_touchControlMap);
}

std::wstring TouchDeviceAndroid::getControlName(int32_t control)
{
	return c_touchControlMap[control].name;
}

bool TouchDeviceAndroid::isControlAnalogue(int32_t control) const
{
	return c_touchControlMap[control].analogue;
}

bool TouchDeviceAndroid::isControlStable(int32_t control) const
{
	return false;
}

float TouchDeviceAndroid::getControlValue(int32_t control)
{
	const TouchControlMap& mc = c_touchControlMap[control];
	if (mc.controlType == DtPositionX)
		return m_positionX[0];
	else if (mc.controlType == DtPositionY)
		return m_positionY[0];
	else if (mc.controlType == DtPositionX2)
		return m_positionX[1];
	else if (mc.controlType == DtPositionY2)
		return m_positionY[1];
	else if (mc.controlType == DtPositionX3)
		return m_positionX[2];
	else if (mc.controlType == DtPositionY3)
		return m_positionY[2];
	else if (mc.controlType == DtPositionX4)
		return m_positionX[3];
	else if (mc.controlType == DtPositionY4)
		return m_positionY[3];
	else if (mc.controlType == DtButton1)
	{
		if (m_activePointerIds.size() >= 1)
			return m_activePointerIds[0].active ? 1.0f : 0.0f;
		else
			return 0.0f;
	}
	else if (mc.controlType == DtButton2)
	{
		if (m_activePointerIds.size() >= 2)
			return m_activePointerIds[1].active ? 1.0f : 0.0f;
		else
			return 0.0f;
	}
	else if (mc.controlType == DtButton3)
	{
		if (m_activePointerIds.size() >= 3)
			return m_activePointerIds[2].active ? 1.0f : 0.0f;
		else
			return 0.0f;
	}
	else if (mc.controlType == DtButton4)
	{
		if (m_activePointerIds.size() >= 4)
			return m_activePointerIds[3].active ? 1.0f : 0.0f;
		else
			return 0.0f;
	}
	else
		return 0.0f;
}

bool TouchDeviceAndroid::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const TouchControlMap& tc = c_touchControlMap[control];
	if (tc.controlType == DtPositionX || tc.controlType == DtPositionX2 || tc.controlType == DtPositionX3 || tc.controlType == DtPositionX4)
	{
		outMin = 0.0f;
		outMax = m_width;
		return true;
	}
	else if (tc.controlType == DtPositionY || tc.controlType == DtPositionY2 || tc.controlType == DtPositionY3 || tc.controlType == DtPositionY4)
	{
		outMin = 0.0f;
		outMax = m_height;
		return true;
	}
	else
		return false;
}

bool TouchDeviceAndroid::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_touchControlMap); ++i)
	{
		const TouchControlMap& tc = c_touchControlMap[i];
		if (tc.controlType == controlType && tc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool TouchDeviceAndroid::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void TouchDeviceAndroid::resetState()
{
	for (int32_t i = 0; i < 3; ++i)
	{
		m_positionX[i] = 0.0f;
		m_positionY[i] = 0.0f;
	}
	m_activePointerIds.clear();
}

void TouchDeviceAndroid::readState()
{
}

bool TouchDeviceAndroid::supportRumble() const
{
	return false;
}

void TouchDeviceAndroid::setRumble(const InputRumble& rumble)
{
}

void TouchDeviceAndroid::setExclusive(bool exclusive)
{
}

void TouchDeviceAndroid::handleInput(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
		return;

	int32_t action = AMotionEvent_getAction(event);
	uint32_t actionEvent = action & AMOTION_EVENT_ACTION_MASK;

	if (actionEvent == AMOTION_EVENT_ACTION_DOWN)
	{
		// Touch begin, first finger.
		int32_t pointerId = AMotionEvent_getPointerId(event, 0);
		T_FATAL_ASSERT (m_activePointerIds.empty());
		m_activePointerIds.push_back({ pointerId, true });
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_UP)
	{
		// Touch end, last finger left.
		m_activePointerIds.clear();
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_POINTER_DOWN)
	{
		// Another finger added.
		int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);

		bool alreadyAdded = false;
		for (size_t i = 0; i < m_activePointerIds.size(); ++i)
		{
			if (m_activePointerIds[i].id == pointerId)
			{
				alreadyAdded = true;
				break;
			}
		}

		if (!alreadyAdded)
		{
			if (!m_activePointerIds.full())
				m_activePointerIds.push_back({ pointerId, true });
		}
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_POINTER_UP)
	{
		int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);

		for (size_t i = 0; i < m_activePointerIds.size(); ++i)
		{
			if (m_activePointerIds[i].id == pointerId)
			{
				m_activePointerIds[i].active = false;
				break;
			}
		}
	}

	int32_t pointerCount = AMotionEvent_getPointerCount(event);
	for (int32_t i = 0; i < pointerCount; ++i)
	{
		int32_t pointerId = AMotionEvent_getPointerId(event, i);
		for (size_t j = 0; j < m_activePointerIds.size(); ++j)
		{
			if (m_activePointerIds[j].id == pointerId)
			{
				m_positionX[j] = AMotionEvent_getX(event, i);
				m_positionY[j]= AMotionEvent_getY(event, i);
				break;
			}
		}
	}
}

	}
}
