#ifndef traktor_online_MatchMaking_H
#define traktor_online_MatchMaking_H

#include "Online/IMatchMaking.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class IUserProvider;
class TaskQueue;

class MatchMaking : public IMatchMaking
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const std::wstring& key, const std::wstring& value);

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers);

private:
	friend class SessionManager;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< IUserProvider > m_userProvider;
	Ref< TaskQueue > m_taskQueue;

	MatchMaking(IMatchMakingProvider* matchMakingProvider, IUserProvider* userProvider, TaskQueue* taskQueue);
};

	}
}


#endif	// traktor_online_MatchMaking_H
