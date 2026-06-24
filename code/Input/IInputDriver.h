/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Platform.h"
#include "Core/Ref.h"
#include "InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class IInputDevice;

/*! Input driver base.
 * \ingroup Input
 */
class T_DLLCLASS IInputDriver : public Object
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrOk = 0,
		UrDevicesChanged = 1,
		UrFailure = -1
	};

	/*!*/
	virtual void destroy() {}

	/*! Create input driver.
	 *
	 * \param sysapp System specific application object.
	 * \param syswin Description of application's system window.
	 * \param inputCategories Input device categories.
	 */
	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) = 0;

	/*! Get number of devices implemented by the driver.
	 *
	 * \note Users should not call this method.
	 *
	 * \return Number of devices.
	 */
	virtual int getDeviceCount() = 0;

	/*! Get device from ordinal.
	 *
	 * \note Users should not call this method.
	 *
	 * \param index Device ordinal; must be in 0 to getDeviceCount()-1 range.
	 * \return Device implementation.
	 */
	virtual Ref< IInputDevice > getDevice(int index) = 0;

	/*! Update driver.
	 *
	 * During driver update devices might be connected or disconnected
	 * thus the driver implementation must return UrDevicesChanged in
	 * such case to let the input system update it's mapping.
	 *
	 * \note Users should not call this method.
	 *
	 * \return Update result.
	 */
	virtual UpdateResult update() = 0;

	/*! Notify driver of the render output (client window) size.
	 *
	 * Used to scale absolute controls such as mouse position when the window
	 * is resized. The default implementation does nothing; drivers that report
	 * absolute, window-relative controls should override it.
	 *
	 * \param width Width of output in pixels.
	 * \param height Height of output in pixels.
	 */
	virtual void setSize(int32_t width, int32_t height) {}
};

}
