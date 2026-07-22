/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <libdecor.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
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

	// Bail out cleanly if the compositor dropped us or a protocol error occurred,
	// rather than silently spinning forever on a dead display.
	if (checkConnectionError())
		return false;

	// Dispatch libdecor events (decoration configure/close/commit).
	if (m_context->getLibdecor())
		libdecor_dispatch(m_context->getLibdecor(), 0);

	// Pre-translate through Application owner, then dispatch to widgets.
	m_context->processEventQueue(owner);

	// Process deferred redraws queued by update(!immediate).
	m_context->processPendingExposes();

	// Run work posted onto the UI thread (see execute() for rationale).
	if (Application* application = Application::getInstance())
		application->flushDeferred();

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

		// Bail out cleanly if the compositor dropped us or a protocol error
		// occurred, rather than silently spinning forever on a dead display.
		if (checkConnectionError())
			break;

		// Dispatch libdecor events.
		if (m_context->getLibdecor())
			libdecor_dispatch(m_context->getLibdecor(), 0);

		// Pre-translate through Application owner, then dispatch to widgets.
		m_context->processEventQueue(owner);

		// Process deferred redraws.
		m_context->processPendingExposes();

		// Run work posted onto the UI thread (e.g. from other threads). Done every
		// iteration, not only on idle, since idle events here fire only after input
		// activity and would otherwise stall deferred work while the app sits idle.
		if (Application* application = Application::getInstance())
			application->flushDeferred();

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

bool EventLoopWl::checkConnectionError()
{
	if (m_terminated)
		return true;

	wl_display* display = m_context->getDisplay();
	const int err = wl_display_get_error(display);
	if (err == 0)
		return false;

	// A non-zero error is unrecoverable: the socket to the compositor is gone.
	// libwayland turns every subsequent call into a no-op, so without this check
	// the process would keep running with no window (the "ghost process" symptom)
	// instead of exiting.
	if (err == EPROTO)
	{
		const wl_interface* iface = nullptr;
		uint32_t id = 0;
		const uint32_t code = wl_display_get_protocol_error(display, &iface, &id);
		log::error << L"Wayland connection lost: protocol error " << code
			<< L" on interface '" << (iface ? mbstows(iface->name) : std::wstring(L"?"))
			<< L"' (object " << id << L"). Terminating UI event loop." << Endl;
	}
	else
		log::error << L"Wayland connection lost: " << mbstows(std::strerror(err))
			<< L" (errno " << err << L"). Terminating UI event loop." << Endl;

	m_exitCode = 1;
	m_terminated = true;
	return true;
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
