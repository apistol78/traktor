/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual bool ready() const override final;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) override final;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers, LobbyAccess access) override final;

	virtual Ref< ILobby > acceptLobby() override final;

	virtual Ref< PartyResult > createParty() override final;

	virtual Ref< IParty > acceptParty() override final;

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
