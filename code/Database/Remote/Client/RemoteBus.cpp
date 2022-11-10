/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Remote/Client/RemoteBus.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/DbmPutEvent.h"
#include "Database/Remote/Messages/DbmGetEvent.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/DbmGetEventResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteBus", RemoteBus, IProviderBus)

RemoteBus::RemoteBus(RemoteConnection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteBus::~RemoteBus()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

bool RemoteBus::putEvent(const IEvent* event)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmPutEvent(m_handle, event));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteBus::getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote)
{
	Ref< DbmGetEventResult > result = m_connection->sendMessage< DbmGetEventResult >(DbmGetEvent(m_handle, inoutSqnr));
	if (!result)
		return false;

	if (!result->getEvent())
		return false;

	inoutSqnr = result->getSequenceNumber();
	outEvent = result->getEvent();
	outRemote = result->getRemote();

	return true;
}

	}
}
