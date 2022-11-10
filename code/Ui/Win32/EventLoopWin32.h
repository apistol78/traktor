/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IEventLoop.h"
#include "Ui/Win32/Window.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class T_DLLCLASS EventLoopWin32 : public IEventLoop
{
public:
	EventLoopWin32();

	virtual ~EventLoopWin32();

	virtual void destroy() override final;

	virtual bool process(EventSubject* owner) override final;

	virtual int32_t execute(EventSubject* owner) override final;

	virtual void exit(int32_t exitCode) override final;

	virtual int32_t getExitCode() const override final;

	virtual int32_t getAsyncKeyState() const override final;

	virtual bool isKeyDown(VirtualKey vk) const override final;

private:
	int32_t m_exitCode;
	bool m_terminate;

	bool preTranslateMessage(EventSubject* owner, const MSG& msg);
};

	}
}

