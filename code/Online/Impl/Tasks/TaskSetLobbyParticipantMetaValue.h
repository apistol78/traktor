#ifndef traktor_online_TaskSetLobbyParticipantMetaValue_H
#define traktor_online_TaskSetLobbyParticipantMetaValue_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class Result;

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_provider;
	uint64_t m_lobbyHandle;
	std::wstring m_key;
	std::wstring m_value;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskSetLobbyParticipantMetaValue_H
