#ifndef traktor_online_CurrentUserSteam_H
#define traktor_online_CurrentUserSteam_H

#include "Online/IUser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class CurrentUserSteam : public IUser
{
	T_RTTI_CLASS;

public:
	CurrentUserSteam();

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool getAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual bool rewardAchievement(IAchievement* achievement);

	virtual bool sendMessage(const std::wstring& message) const;
};

	}
}

#endif	// traktor_online_CurrentUserSteam_H
