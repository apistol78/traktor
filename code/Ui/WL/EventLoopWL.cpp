/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/WL/EventLoopWL.h"

namespace traktor::ui
{

EventLoopWL::EventLoopWL(Context* context)
:   m_context(context)
{
}

EventLoopWL::~EventLoopWL()
{
}

void EventLoopWL::destroy()
{
	delete this;
}

bool EventLoopWL::process(EventSubject* owner)
{
	return false;
}

int32_t EventLoopWL::execute(EventSubject* owner)
{
	return 0;
}

void EventLoopWL::exit(int32_t exitCode)
{
}

int32_t EventLoopWL::getExitCode() const
{
	return 0;
}

int32_t EventLoopWL::getAsyncKeyState() const
{
	return KsNone;
}

bool EventLoopWL::isKeyDown(VirtualKey vk) const
{
	return false;
}

}
