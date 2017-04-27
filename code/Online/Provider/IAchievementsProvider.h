/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_IAchievementsProvider_H
#define traktor_online_IAchievementsProvider_H

#include <map>
#include "Core/Object.h"

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

class T_DLLCLASS IAchievementsProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) = 0;

	virtual bool set(const std::wstring& achievementId, bool reward) = 0;
};

	}
}

#endif	// traktor_online_IAchievementsProvider_H
