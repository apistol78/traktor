#ifndef traktor_online_MatchMaking_H
#define traktor_online_MatchMaking_H

#include "Online/IMatchMaking.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class TaskQueue;
class UserCache;

class MatchMaking : public IMatchMaking
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) T_OVERRIDE T_FINAL;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers, LobbyAccess access) T_OVERRIDE T_FINAL;

	virtual Ref< ILobby > acceptLobby() T_OVERRIDE T_FINAL;

	virtual Ref< PartyResult > createParty() T_OVERRIDE T_FINAL;

	virtual Ref< IParty > acceptParty() T_OVERRIDE T_FINAL;

private:
	friend class SessionManager;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;

	MatchMaking(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue);
};

	}
}


#endif	// traktor_online_MatchMaking_H
