/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/UserArrayResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserArrayResult", UserArrayResult, Result)

void UserArrayResult::succeed(const RefArray< IUser >& users)
{
	m_users = users;
	Result::succeed();
}

const RefArray< IUser >& UserArrayResult::get() const
{
	wait();
	return m_users;
}

	}
}
