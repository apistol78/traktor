/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LobbyArrayResult_H
#define traktor_online_LobbyArrayResult_H

#include "Core/RefArray.h"
#include "Core/Thread/Result.h"

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

class T_DLLCLASS LobbyArrayResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(const RefArray< ILobby >& lobbies);

	const RefArray< ILobby >& get() const;

private:
	RefArray< ILobby > m_lobbies;
};

	}
}

#endif	// traktor_online_LobbyArrayResult_H
