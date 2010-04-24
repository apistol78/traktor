#ifndef traktor_online_SessionSteam_H
#define traktor_online_SessionSteam_H

#include "Online/ISession.h"

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

class CurrentUserSteam;

class T_DLLCLASS SessionSteam : public ISession
{
	T_RTTI_CLASS;

public:
	SessionSteam(CurrentUserSteam* user);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool rewardAchievement(const std::wstring& achievementId);

	virtual bool withdrawAchievement(const std::wstring& achievementId);

	virtual bool setStatValue(const std::wstring& statId, float value);

	virtual bool getStatValue(const std::wstring& statId, float& outValue);

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

	bool update();

	STEAM_CALLBACK(SessionSteam, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);

	STEAM_CALLBACK(SessionSteam, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);

	STEAM_CALLBACK(SessionSteam, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

private:
	Ref< CurrentUserSteam > m_user;
	bool m_requestedStats;
	bool m_receivedStats;
	bool m_storeStats;
	bool m_destroyed;
};

	}
}

#endif	// traktor_online_SessionSteam_H
