/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <android/input.h>
#include <android/native_window.h>
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Math/MathUtils.h"
#include "Input/Android/GamepadDeviceAndroid.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct ControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	bool stable;
}
c_controlMap[] =
{
	{ L"Left X axis", DtThumbLeftX, true, true },
	{ L"Left Y axis", DtThumbLeftY, true, true },
	{ L"Right X axis", DtThumbRightX, true, true },
	{ L"Right Y axis", DtThumbRightY, true, true }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GamepadDeviceAndroid", GamepadDeviceAndroid, IInputDevice)

GamepadDeviceAndroid::GamepadDeviceAndroid(const SystemWindow& syswin)
:	m_syswim(syswin)
{
	resetState();
}

std::wstring GamepadDeviceAndroid::getName() const
{
	return L"Gamepad";
}

InputCategory GamepadDeviceAndroid::getCategory() const
{
	return CtJoystick;
}

bool GamepadDeviceAndroid::isConnected() const
{
	return true;
}

int32_t GamepadDeviceAndroid::getControlCount()
{
	return sizeof_array(c_controlMap);
}

std::wstring GamepadDeviceAndroid::getControlName(int32_t control)
{
	return c_controlMap[control].name;
}

bool GamepadDeviceAndroid::isControlAnalogue(int32_t control) const
{
	return c_controlMap[control].analogue;
}

bool GamepadDeviceAndroid::isControlStable(int32_t control) const
{
	return c_controlMap[control].stable;
}

float GamepadDeviceAndroid::getControlValue(int32_t control)
{
	const ControlMap& mc = c_controlMap[control];
	if (mc.controlType == DtThumbLeftX)
	 	return m_thumbLeftAxisX;
	else if (mc.controlType == DtThumbLeftY)
	 	return m_thumbLeftAxisY;
	else if (mc.controlType == DtThumbRightX)
		return m_thumbRightAxisX;
	else if (mc.controlType == DtThumbRightY)
	 	return m_thumbRightAxisY;
	else
		return 0.0f;
}

bool GamepadDeviceAndroid::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = -1.0f;
	outMax = 1.0f;
	return true;
}

bool GamepadDeviceAndroid::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_controlMap); ++i)
	{
		const ControlMap& mc = c_controlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool GamepadDeviceAndroid::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void GamepadDeviceAndroid::resetState()
{
	m_pointers.clear();
	m_thumbLeftPointerId = -1;
	m_thumbLeftCenter = Vector2(0.0f, 0.0f);
	m_thumbRightPointerId = -1;
	m_thumbRightCenter = Vector2(0.0f, 0.0f);
	m_thumbLeftAxisX = 0.0f;
	m_thumbLeftAxisY = 0.0f;
	m_thumbRightAxisX = 0.0f;
	m_thumbRightAxisY = 0.0f;
}

void GamepadDeviceAndroid::readState()
{
	int32_t width = ANativeWindow_getWidth(*m_syswim.window);
	int32_t height = ANativeWindow_getHeight(*m_syswim.window);

	float slabW = width / 3.0f;
	float slabH = height / 2.0f;
	float padRadius = std::min(slabW, slabH);

	// Update left pad.
	if (m_thumbLeftPointerId >= 0)
	{
		auto it = m_pointers.find(m_thumbLeftPointerId);
		if (it != m_pointers.end())
		{
			Vector2 position(it->second.positionX, it->second.positionY);
			Vector2 delta = position - m_thumbLeftCenter;
			m_thumbLeftAxisX = clamp( delta.x / padRadius, -1.0f, 1.0f);
			m_thumbLeftAxisY = clamp(-delta.y / padRadius, -1.0f, 1.0f);
		}
		else
		{
			m_thumbLeftPointerId = -1;
			m_thumbLeftAxisX = 0.0f;
			m_thumbLeftAxisY = 0.0f;
		}
	}
	else
	{
		Vector2 leftCenter(slabW / 2.0f, slabH + slabH / 2.0f);
		for (auto& it : m_pointers)
		{
			Vector2 position(it.second.positionX, it.second.positionY);
			Vector2 delta = position - leftCenter;
			if (delta.length() <= padRadius)
			{
				m_thumbLeftPointerId = it.first;
				m_thumbLeftCenter = position;
				break;
			}
		}
	}

	// Update right pad.
	if (m_thumbRightPointerId >= 0)
	{
		auto it = m_pointers.find(m_thumbRightPointerId);
		if (it != m_pointers.end())
		{
			Vector2 position(it->second.positionX, it->second.positionY);
			Vector2 delta = position - m_thumbRightCenter;
			m_thumbRightAxisX = clamp( delta.x / padRadius, -1.0f, 1.0f);
			m_thumbRightAxisY = clamp(-delta.y / padRadius, -1.0f, 1.0f);
		}
		else
		{
			m_thumbRightPointerId = -1;
			m_thumbRightAxisX = 0.0f;
			m_thumbRightAxisY = 0.0f;
		}
	}
	else
	{
		Vector2 rightCenter(slabW * 2.0f + slabW / 2.0f, slabH + slabH / 2.0f);
		for (auto& it : m_pointers)
		{
			Vector2 position(it.second.positionX, it.second.positionY);
			Vector2 delta = position - rightCenter;
			if (delta.length() <= padRadius)
			{
				m_thumbRightPointerId = it.first;
				m_thumbRightCenter = position;
				break;
			}
		}
	}
}

bool GamepadDeviceAndroid::supportRumble() const
{
	return false;
}

void GamepadDeviceAndroid::setRumble(const InputRumble& /*rumble*/)
{
}

void GamepadDeviceAndroid::setExclusive(bool exclusive)
{
}

void GamepadDeviceAndroid::handleInput(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
		return;

	int32_t action = AMotionEvent_getAction(event);
	uint32_t actionEvent = action & AMOTION_EVENT_ACTION_MASK;

	if (actionEvent == AMOTION_EVENT_ACTION_DOWN)
	{
		// Touch begin, first finger.
		int32_t pointerId = AMotionEvent_getPointerId(event, 0);
		T_FATAL_ASSERT (m_pointers.empty());
		m_pointers[pointerId] = { 0.0f, 0.0f };
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_UP)
	{
		// Touch end, last finger left.
		m_pointers.clear();
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_POINTER_DOWN)
	{
		// Another finger added.
		int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
		m_pointers[pointerId] = { 0.0f, 0.0f };
	}
	else if (actionEvent == AMOTION_EVENT_ACTION_POINTER_UP)
	{
		int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
		m_pointers.remove(pointerId);
	}

	// Update positions of active pointers.
	int32_t pointerCount = AMotionEvent_getPointerCount(event);
	for (int32_t i = 0; i < pointerCount; ++i)
	{
		int32_t pointerId = AMotionEvent_getPointerId(event, i);
		auto it = m_pointers.find(pointerId);
		if (it != m_pointers.end())
		{
			it->second.positionX = AMotionEvent_getX(event, i);
			it->second.positionY = AMotionEvent_getY(event, i);
		}
	}
}

	}
}
