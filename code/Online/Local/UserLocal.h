#ifndef traktor_online_UserLocal_H
#define traktor_online_UserLocal_H

#include "Online/IUser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserLocal : public IUser
{
	T_RTTI_CLASS(UserLocal)

public:
	UserLocal(const std::wstring& name);

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool getAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual bool rewardAchievement(IAchievement* achievement);

	virtual bool sendMessage(const std::wstring& message) const;

private:
	std::wstring m_name;
	RefArray< IAchievement > m_achievements;
};

	}
}

#endif	// traktor_online_UserLocal_H
