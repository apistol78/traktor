/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_IMatchMaking_H
#define traktor_online_IMatchMaking_H

#include "Online/LobbyResult.h"
#include "Online/LobbyArrayResult.h"
#include "Online/PartyResult.h"
#include "Online/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class ILobby;
class IParty;
class LobbyFilter;

class T_DLLCLASS IMatchMaking : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) = 0;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers, LobbyAccess access) = 0;

	virtual Ref< ILobby > acceptLobby() = 0;

	virtual Ref< PartyResult > createParty() = 0;

	virtual Ref< IParty > acceptParty() = 0;
};

	}
}


#endif	// traktor_online_IMatchMaking_H
