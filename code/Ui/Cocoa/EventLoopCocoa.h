/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_COCOA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if __OBJC__
@class NSEvent;
@class NSWindow;
#else
typedef void NSEvent;
typedef void NSWindow;
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopCocoa : public IEventLoop
{
public:
	EventLoopCocoa();

	virtual ~EventLoopCocoa();

	virtual void destroy() override final;

	virtual bool process(EventSubject* owner) override final;

	virtual int32_t execute(EventSubject* owner) override final;

	virtual void exit(int32_t exitCode) override final;

	virtual int32_t getExitCode() const override final;

	virtual int32_t getAsyncKeyState() const override final;

	virtual bool isKeyDown(VirtualKey vk) const override final;

	void pushModal(NSWindow* modalWindow);

	void popModal();

private:
	bool m_launching;
	int32_t m_exitCode;
	bool m_terminated;
	uint32_t m_modifierFlags;
	bool m_idleMode;
	std::vector< NSWindow* > m_modalWindows;

	bool handleGlobalEvents(EventSubject* owner, NSEvent* event);
};

	}
}

