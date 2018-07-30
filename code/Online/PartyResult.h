/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_PartyResult_H
#define traktor_online_PartyResult_H

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

class IParty;

class T_DLLCLASS PartyResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(IParty* lobby);

	IParty* get() const;

private:
	Ref< IParty > m_lobby;
};

	}
}

#endif	// traktor_online_PartyResult_H
