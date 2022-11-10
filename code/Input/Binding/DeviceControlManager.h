/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Input/InputTypes.h"

namespace traktor
{
	namespace input
	{

class DeviceControl;
class InputSystem;

/*! Device control manager.
 * \ingroup Input
 */
class DeviceControlManager : public Object
{
	T_RTTI_CLASS;

public:
	DeviceControlManager(InputSystem* inputSystem);

	int32_t getDeviceControlCount(InputCategory category);

	Ref< DeviceControl > getDeviceControl(InputCategory category, InputDefaultControlType controlType, bool analogue, int32_t index);

	void update();

	InputSystem* getInputSystem() { return m_inputSystem; }

private:
	Ref< InputSystem > m_inputSystem;
	RefArray< DeviceControl > m_deviceControls;
};

	}
}

