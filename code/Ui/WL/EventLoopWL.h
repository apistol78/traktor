/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IEventLoop.h"
#include "Ui/WL/Context.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class T_DLLCLASS EventLoopWL : public IEventLoop
{
public:
	explicit EventLoopWL(Context* context);

	virtual ~EventLoopWL();

	virtual void destroy() override final;

	virtual bool process(EventSubject* owner) override final;

	virtual int32_t execute(EventSubject* owner) override final;

	virtual void exit(int32_t exitCode) override final;

	virtual int32_t getExitCode() const override final;

	virtual int32_t getAsyncKeyState() const override final;

	virtual bool isKeyDown(VirtualKey vk) const override final;

private:
	Context* m_context = nullptr;
};

}
