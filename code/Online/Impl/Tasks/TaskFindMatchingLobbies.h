#ifndef traktor_online_TaskFindMatchingLobbies_H
#define traktor_online_TaskFindMatchingLobbies_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class LobbyArrayResult;
class UserCache;

class TaskFindMatchingLobbies : public ITask
{
	T_RTTI_CLASS;

public:
	TaskFindMatchingLobbies(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		const std::wstring& key,
		const std::wstring& value,
		LobbyArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	std::wstring m_key;
	std::wstring m_value;
	Ref< LobbyArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskFindMatchingLobbies_H
