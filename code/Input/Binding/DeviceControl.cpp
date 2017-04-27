/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/IInputDevice.h"
#include "Input/Binding/DeviceControl.h"

namespace traktor
{
	namespace input
	{

std::wstring DeviceControl::getControlName() const
{
	if (m_device && m_device->isConnected())
		return m_device->getControlName(m_control);
	else
		return L"";
}

InputCategory DeviceControl::getCategory() const
{
	return m_category;
}

InputDefaultControlType DeviceControl::getControlType() const
{
	return m_controlType;
}
	
bool DeviceControl::isAnalogue() const
{
	return m_analogue;
}
	
int32_t DeviceControl::getIndex() const
{
	return m_index;
}

IInputDevice* DeviceControl::getDevice() const
{
	return m_device;
}

float DeviceControl::getRangeMin() const
{
	return m_rangeMin;
}

float DeviceControl::getRangeMax() const
{
	return m_rangeMax;
}

float DeviceControl::getPreviousValue() const
{
	return m_previousValue;
}

float DeviceControl::getCurrentValue() const
{
	return m_currentValue;
}

DeviceControl::DeviceControl()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_analogue(false)
,	m_index(0)
,	m_control(0)
,	m_rangeMin(0.0f)
,	m_rangeMax(0.0f)
,	m_previousValue(0.0f)
,	m_currentValue(0.0f)
{
}

	}
}
