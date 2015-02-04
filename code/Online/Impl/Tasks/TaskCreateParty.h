#ifndef traktor_online_TaskCreateParty_H
#define traktor_online_TaskCreateParty_H

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class PartyResult;
class UserCache;

class TaskCreateParty : public ITask
{
	T_RTTI_CLASS;

public:
	TaskCreateParty(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		PartyResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< PartyResult > m_result;
};

	}
}

#endif	// traktor_online_TaskCreateParty_H
