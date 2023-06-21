/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "Core/Timer/Timer.h"
#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_X11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Assoc;

class T_DLLCLASS EventLoopX11 : public IEventLoop
{
public:
	explicit EventLoopX11(Context* context);

	virtual ~EventLoopX11();

	virtual void destroy() override final;

	virtual bool process(EventSubject* owner) override final;

	virtual int32_t execute(EventSubject* owner) override final;

	virtual void exit(int32_t exitCode) override final;

	virtual int32_t getExitCode() const override final;

	virtual int32_t getAsyncKeyState() const override final;

	virtual bool isKeyDown(VirtualKey vk) const override final;

private:
	Ref< Context > m_context;
	XIC m_xic;
	Timer m_timer;
	bool m_terminated;
	int32_t m_exitCode;
	int32_t m_keyState;

	bool preTranslateEvent(EventSubject* owner, XEvent& e);
};

}
