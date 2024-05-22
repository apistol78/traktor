/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Event.h"
#include "Ui/Itf/IEventLoop.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Event", Event, Object)

Event::Event(EventSubject* sender)
:	m_sender(sender)
,	m_consumed(false)
{
}

EventSubject* Event::getSender() const
{
	return m_sender;
}

int32_t Event::getKeyState() const
{
	auto application = Application::getInstance();
	if (application != nullptr && application->getEventLoop() != nullptr)
		return application->getEventLoop()->getAsyncKeyState();
	else
		return WsNone;
}

void Event::consume()
{
	m_consumed = true;
}

bool Event::consumed() const
{
	return m_consumed;
}

}
