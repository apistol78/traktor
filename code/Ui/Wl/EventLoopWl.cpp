/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <poll.h>
#include <libdecor.h>
#include "Core/Log/Log.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/EventLoopWl.h"
#include "Ui/Wl/Timers.h"

namespace traktor::ui
{

EventLoopWl::EventLoopWl(ContextWl* context)
:	m_context(context)
,	m_terminated(false)
,	m_exitCode(0)
{
}

EventLoopWl::~EventLoopWl()
{
}

void EventLoopWl::destroy()
{
	delete this;
}

bool EventLoopWl::process(EventSubject* owner)
{
	if (m_terminated)
		return false;

	// Flush outgoing requests.
	wl_display_flush(m_context->getDisplay());

	// Dispatch already-queued Wayland events (calls our listener callbacks
	// which enqueue WlEvents into the context's event queue).
	while (wl_display_prepare_read(m_context->getDisplay()) != 0)
		wl_display_dispatch_pending(m_context->getDisplay());

	// Poll for new data from the compositor.
	pollfd pfd;
	pfd.fd = wl_display_get_fd(m_context->getDisplay());
	pfd.events = POLLIN;
	pfd.revents = 0;

	if (poll(&pfd, 1, 1) > 0)
		wl_display_read_events(m_context->getDisplay());
	else
		wl_display_cancel_read(m_context->getDisplay());

	// Dispatch newly-read events (again populates the context event queue).
	wl_display_dispatch_pending(m_context->getDisplay());

	// Dispatch libdecor events (decoration configure/close/commit).
	if (m_context->getLibdecor())
		libdecor_dispatch(m_context->getLibdecor(), 0);

	// Pre-translate through Application owner, then dispatch to widgets.
	m_context->processEventQueue(owner);

	// Process deferred redraws queued by update(!immediate).
	m_context->processPendingExposes();

	const double dt = m_timer.getDeltaTime();
	Timers::getInstance().update(dt);

	return !m_terminated;
}

int32_t EventLoopWl::execute(EventSubject* owner)
{
	bool idle = true;

	while (!m_terminated)
	{
		// Flush outgoing requests.
		wl_display_flush(m_context->getDisplay());

		// Dispatch already-queued Wayland events.
		while (wl_display_prepare_read(m_context->getDisplay()) != 0)
			wl_display_dispatch_pending(m_context->getDisplay());

		// Poll for new data from the compositor.
		pollfd pfd;
		pfd.fd = wl_display_get_fd(m_context->getDisplay());
		pfd.events = POLLIN;
		pfd.revents = 0;

		int ret = poll(&pfd, 1, 1);
		if (ret > 0)
		{
			wl_display_read_events(m_context->getDisplay());
			idle = true;
		}
		else
			wl_display_cancel_read(m_context->getDisplay());

		// Dispatch newly-read events.
		wl_display_dispatch_pending(m_context->getDisplay());

		// Dispatch libdecor events.
		if (m_context->getLibdecor())
			libdecor_dispatch(m_context->getLibdecor(), 0);

		// Pre-translate through Application owner, then dispatch to widgets.
		m_context->processEventQueue(owner);

		// Process deferred redraws.
		m_context->processPendingExposes();

		if (idle)
		{
			IdleEvent idleEvent(owner);
			owner->raiseEvent(&idleEvent);
			if (!idleEvent.requestedMore())
				idle = false;
		}

		const double dt = m_timer.getDeltaTime();
		Timers::getInstance().update(dt);
	}

	return m_exitCode;
}

void EventLoopWl::exit(int32_t exitCode)
{
	m_exitCode = exitCode;
	m_terminated = true;
}

int32_t EventLoopWl::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopWl::getAsyncKeyState() const
{
	int32_t keyState = KsNone;

	xkb_state* xkbState = m_context->getXkbState();
	if (xkbState)
	{
		if (xkb_state_mod_name_is_active(xkbState, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE))
			keyState |= KsShift;
		if (xkb_state_mod_name_is_active(xkbState, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
			keyState |= KsControl | KsCommand;
		if (xkb_state_mod_name_is_active(xkbState, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
			keyState |= KsMenu;
	}

	return keyState;
}

bool EventLoopWl::isKeyDown(VirtualKey vk) const
{
	return false;
}

}
