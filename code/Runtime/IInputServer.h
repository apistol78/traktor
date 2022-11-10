/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/InputTypes.h"
#include "Runtime/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputMapping;
class InputMappingStateData;
class InputSystem;
class RumbleEffectPlayer;

	}

	namespace runtime
	{

/*! Input server.
 * \ingroup Runtime
 *
 * "Input.Rumble"	- Rumble enable.
 */
class T_DLLCLASS IInputServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual bool createInputMapping(const input::InputMappingStateData* stateData) = 0;

	virtual bool fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue) = 0;

	virtual bool isFabricating() const = 0;

	virtual bool abortedFabricating() const = 0;

	virtual bool resetInputSource(const std::wstring& sourceId) = 0;

	virtual bool isIdle() const = 0;

	virtual void apply() = 0;

	virtual void revert() = 0;

	virtual input::InputSystem* getInputSystem() = 0;

	virtual input::InputMapping* getInputMapping() = 0;

	virtual input::RumbleEffectPlayer* getRumbleEffectPlayer() = 0;
};

	}
}

