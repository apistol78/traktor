#pragma once

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class IMatchMakingProvider;

class TaskSetLobbyParticipantMetaValue : public ITask
{
	T_RTTI_CLASS;

public:
	TaskSetLobbyParticipantMetaValue(
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
}

