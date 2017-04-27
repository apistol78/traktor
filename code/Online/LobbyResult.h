/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LobbyResult_H
#define traktor_online_LobbyResult_H

#include "Online/Result.h"

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

class T_DLLCLASS LobbyResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(ILobby* lobby);

	ILobby* get() const;

private:
	Ref< ILobby > m_lobby;
};

	}
}

#endif	// traktor_online_LobbyResult_H
