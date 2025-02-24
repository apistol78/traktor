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
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct android_app;
struct AInputEvent;
struct ANativeActivity;

namespace traktor
{

/*! Android delegable instance.
 * \ingroup Core
 */
class T_DLLCLASS DelegateInstance
{
public:
	struct IDelegate
	{
		virtual void notifyHandleCommand(DelegateInstance* instance, int32_t cmd) {}

		virtual void notifyHandleInput(DelegateInstance* instance, AInputEvent* event) {}

		virtual void notifyHandleEvents(DelegateInstance* instance) {}
	};

	virtual struct android_app* getApplication() = 0;

	virtual struct ANativeActivity* getActivity() = 0;

	void addDelegate(IDelegate* delegate);

	void removeDelegate(IDelegate* delegate);

	virtual void handleCommand(int32_t cmd);

	virtual void handleInput(AInputEvent* event);

	virtual void handleEvents();

private:
	std::vector< IDelegate* > m_delegates;
};

}

