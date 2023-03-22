/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Result.h"
#include "Online/Impl/Tasks/TaskSetLobbyParticipantMetaValue.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskSetLobbyParticipantMetaValue", TaskSetLobbyParticipantMetaValue, ITask)

TaskSetLobbyParticipantMetaValue::TaskSetLobbyParticipantMetaValue(
	IMatchMakingProvider* provider,
	uint64_t lobbyHandle,
	const std::wstring& key,
	const std::wstring& value,
	Result* result
)
:	m_provider(provider)
,	m_lobbyHandle(lobbyHandle)
,	m_key(key)
,	m_value(value)
,	m_result(result)
{
}

void TaskSetLobbyParticipantMetaValue::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);

	if (m_provider->setLobbyParticipantMetaValue(m_lobbyHandle, m_key, m_value))
		m_result->succeed();
	else
		m_result->fail();
}

}
