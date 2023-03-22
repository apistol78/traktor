/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

}

namespace traktor::online
{

class IMatchMakingProvider;

class TaskSetLobbyMetaValue : public ITask
{
	T_RTTI_CLASS;

public:
	explicit TaskSetLobbyMetaValue(
		IMatchMakingProvider* provider,
		uint64_t lobbyHandle,
		const std::wstring& key,
		const std::wstring& value,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_provider;
	uint64_t m_lobbyHandle;
	std::wstring m_key;
	std::wstring m_value;
	Ref< Result > m_result;
};

}
