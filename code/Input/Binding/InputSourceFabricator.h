/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include "Core/Object.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class KeyboardInputSourceData;
class CombinedInputSourceData;
class IInputDevice;
class IInputSourceData;
class InputSystem;

/*!
 * \ingroup Input
 */
class T_DLLCLASS InputSourceFabricator : public Object
{
	T_RTTI_CLASS;

public:
	explicit InputSourceFabricator(InputSystem* inputSystem, InputCategory category, bool analogue);

	Ref< IInputSourceData > update();

private:
	struct DeviceState
	{
		Ref< IInputDevice > device;
		std::map< DefaultControl, float > values;
	};

	InputCategory m_category;
	bool m_analogue;
	Ref< KeyboardInputSourceData > m_keyboardData;
	Ref< CombinedInputSourceData > m_combinedData;
	Ref< IInputSourceData > m_outputData;
	std::list< DeviceState > m_deviceStates;
};

}
