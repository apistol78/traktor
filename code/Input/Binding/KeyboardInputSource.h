/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class DeviceControl;
class DeviceControlManager;
class IInputDevice;
class InputSystem;

/*!
 * \ingroup Input
 */
class T_DLLCLASS KeyboardInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	explicit KeyboardInputSource(
		const AlignedVector< DefaultControl >& controlTypes,
		DeviceControlManager* deviceControlManager
	);

	virtual ~KeyboardInputSource();

	virtual std::wstring getDescription() const override final;

	virtual void prepare(float T, float dT) override final;

	virtual float read(float T, float dT) override final;

private:
	struct Key
	{
		DefaultControl controlType;
		RefArray< DeviceControl > deviceControls;
		bool state;

		bool operator == (const Key& rh) const;
	};

	Ref< DeviceControlManager > m_deviceControlManager;
	int32_t m_deviceCount;
	AlignedVector< Key > m_keys;
	bool m_state;
};

}
