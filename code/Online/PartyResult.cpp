/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/PartyResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PartyResult", PartyResult, Result)

void PartyResult::succeed(IParty* lobby)
{
	m_lobby = lobby;
	Result::succeed();
}

IParty* PartyResult::get() const
{
	wait();
	return m_lobby;
}

	}
}
