/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/ComRef.h"
#include "Input/IInputDevice.h"

#include <GameInput.h>
#include <windows.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_GAMEINPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*! GameInput mouse, merges all connected mice.
 * \ingroup Input
 */
class T_DLLCLASS MouseDeviceGameInput : public IInputDevice
{
	T_RTTI_CLASS;

public:
	explicit MouseDeviceGameInput(IGameInput* gameInput, HWND hWnd);

	virtual void destroy() override final;

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual bool isControlStable(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

	virtual void setExclusive(bool exclusive) override final;

	void addDevice(IGameInputDevice* device);

private:
	struct Mouse
	{
		ComRef< IGameInputDevice > device;
		int64_t positionX = 0;
		int64_t positionY = 0;
		int64_t wheelY = 0;
		bool valid = false;
	};

	ComRef< IGameInput > m_gameInput;
	AlignedVector< Mouse > m_mice;
	HWND m_hWnd;
	bool m_connected = false;
	bool m_exclusive = false;
	bool m_clipped = false;
	uint64_t m_connectedTimestamp = 0;
	GameInputMouseButtons m_buttons = GameInputMouseNone;
	int64_t m_deltaX = 0;
	int64_t m_deltaY = 0;
	int64_t m_deltaWheel = 0;
	POINT m_position = { 0, 0 };
	RECT m_rect = { 0, 0, 0, 0 };
};

}
