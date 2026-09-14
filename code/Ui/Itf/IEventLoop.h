/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/IRefCount.h"
#include "Ui/Enums.h"
#include "Ui/Size.h"

namespace traktor::ui
{

class EventSubject;

/*! EventLoop interface.
 * \ingroup UI
 */
class IEventLoop : public IRefCount
{
public:
	virtual ~IEventLoop() {}

	virtual void destroy() = 0;

	virtual bool process(EventSubject* owner) = 0;

	virtual int32_t execute(EventSubject* owner) = 0;

	virtual void exit(int32_t exitCode) = 0;

	virtual int32_t getExitCode() const = 0;

	virtual int32_t getAsyncKeyState() const = 0;

	virtual bool isKeyDown(VirtualKey vk) const = 0;

	/*! \name Callback timers.
	 *
	 * Timers driven by the event loop; the callback fires on the UI thread
	 * until stopped. Virtual widget timers are multiplexed through these, so
	 * they must be overridden by any backend hosting virtual widgets; the
	 * defaults cover backends that fall back to native widgets (Cocoa).
	 */
	//@{

	virtual int32_t startTimer(int32_t interval, const std::function< void() >& fn) { return -1; }

	virtual void stopTimer(int32_t id) {}

	//@}
};

}
