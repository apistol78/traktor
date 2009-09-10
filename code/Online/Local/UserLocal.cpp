#include "Online/Local/UserLocal.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserLocal", UserLocal, IUser)

UserLocal::UserLocal(const std::wstring& name)
:	m_name(name)
{
}

std::wstring UserLocal::getName() const
{
	return m_name;
}

bool UserLocal::getFriends(RefArray< IUser >& outFriends) const
{
	return true;
}

bool UserLocal::getAchievements(RefArray< IAchievement >& outAchievements) const
{
	outAchievements = m_achievements;
	return true;
}

bool UserLocal::rewardAchievement(IAchievement* achievement)
{
	m_achievements.push_back(achievement);
	return true;
}

bool UserLocal::sendMessage(const std::wstring& message) const
{
	return false;
}

	}
}
