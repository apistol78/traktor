/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_UserArrayResult_H
#define traktor_online_UserArrayResult_H

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

class IUser;

class T_DLLCLASS UserArrayResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(const RefArray< IUser >& users);

	const RefArray< IUser >& get() const;

private:
	RefArray< IUser > m_users;
};

	}
}

#endif	// traktor_online_UserArrayResult_H
