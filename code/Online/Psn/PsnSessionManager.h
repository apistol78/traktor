#ifndef traktor_online_PsnSessionManager_H
#define traktor_online_PsnSessionManager_H

#include <np.h>
#include "Online/Psn/PsnTypes.h"
#include "Online/Provider/ISessionManagerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_PSN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class PsnAchievements;
class PsnLeaderboards;
class PsnSaveData;
class PsnStatistics;

class T_DLLCLASS PsnSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	PsnSessionManager();

	virtual bool create(const IGameConfiguration* configuration);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireFullScreen() const;

	virtual bool requireUserAttention() const;

	virtual uint64_t getCurrentUserHandle() const;

	virtual bool haveP2PData() const;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const;

	virtual IAchievementsProvider* getAchievements() const;

	virtual ILeaderboardsProvider* getLeaderboards() const;

	virtual IMatchMakingProvider* getMatchMaking() const;

	virtual ISaveDataProvider* getSaveData() const;

	virtual IStatisticsProvider* getStatistics() const;

	virtual IUserProvider* getUser() const;

	virtual IVideoSharingProvider* getVideoSharing() const;

	virtual IVoiceChatProvider* getVoiceChat() const;

private:
	SceNpTrophyContext m_trophyContext;
	SceNpTrophyHandle m_trophyHandle;
	Ref< PsnAchievements > m_achievements;
	Ref< PsnLeaderboards > m_leaderboards;
	Ref< PsnSaveData > m_saveData;
	Ref< PsnStatistics > m_statistics;
	bool m_connected;
	static bool ms_requireUserAttention;

	static void systemCallback(uint64_t status, uint64_t param);
};

	}
}

#endif	// traktor_online_PsnSessionManager_H
