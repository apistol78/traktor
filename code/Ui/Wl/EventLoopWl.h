/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "Core/Timer/Timer.h"
#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class ContextWl;

class T_DLLCLASS EventLoopWl : public RefCountImpl< IEventLoop >
{
public:
	explicit EventLoopWl(ContextWl* context);

	virtual ~EventLoopWl();

	virtual void destroy() override final;

	virtual bool process(EventSubject* owner) override final;

	virtual int32_t execute(EventSubject* owner) override final;

	virtual void exit(int32_t exitCode) override final;

	virtual int32_t getExitCode() const override final;

	virtual int32_t getAsyncKeyState() const override final;

	virtual bool isKeyDown(VirtualKey vk) const override final;

private:
	Ref< ContextWl > m_context;
	Timer m_timer;
	bool m_terminated;
	int32_t m_exitCode;
};

}
