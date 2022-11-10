/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/IRefCount.h"
#include "Core/Ref.h"
#include "Input/InputTypes.h"

namespace traktor
{
	namespace input
	{

class IInputDevice;

/*! Device control mapping to driver and device.
 * \ingroup Input
 */
class DeviceControl : public RefCountImpl< IRefCount >
{
public:
	std::wstring getControlName() const;

	InputCategory getCategory() const;

	InputDefaultControlType getControlType() const;

	bool isAnalogue() const;

	int32_t getIndex() const;

	IInputDevice* getDevice() const;

	float getRangeMin() const;

	float getRangeMax() const;

	float getPreviousValue() const;

	float getCurrentValue() const;

private:
	friend class DeviceControlManager;

	DeviceControl();

	InputCategory m_category;
	InputDefaultControlType m_controlType;
	bool m_analogue;
	int32_t m_index;
	Ref< IInputDevice > m_device;
	int32_t m_control;
	float m_rangeMin;
	float m_rangeMax;
	float m_previousValue;
	float m_currentValue;
};

	}
}

