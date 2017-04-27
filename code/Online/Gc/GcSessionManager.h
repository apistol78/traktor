/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_GcSessionManager_H
#define traktor_online_GcSessionManager_H

#include "Core/Ref.h"
#include "Online/Provider/ISessionManagerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_GC_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class GcAchievements;
class GcLeaderboards;
class GcMatchMaking;
class GcSaveData;
class GcStatistics;
class GcUser;
class GcVideoSharing;

class T_DLLCLASS GcSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	virtual bool create(const IGameConfiguration* configuration) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool update() T_OVERRIDE T_FINAL;

	virtual std::wstring getLanguageCode() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual bool requireFullScreen() const T_OVERRIDE T_FINAL;

	virtual bool requireUserAttention() const T_OVERRIDE T_FINAL;

	virtual bool haveDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool buyDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool navigateUrl(const net::Url& url) const T_OVERRIDE T_FINAL;

	virtual uint64_t getCurrentUserHandle() const T_OVERRIDE T_FINAL;

	virtual bool getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const T_OVERRIDE T_FINAL;

	virtual bool findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const T_OVERRIDE T_FINAL;

	virtual bool haveP2PData() const T_OVERRIDE T_FINAL;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const T_OVERRIDE T_FINAL;

	virtual uint32_t getCurrentGameCount() const T_OVERRIDE T_FINAL;

	virtual IAchievementsProvider* getAchievements() const T_OVERRIDE T_FINAL;

	virtual ILeaderboardsProvider* getLeaderboards() const T_OVERRIDE T_FINAL;

	virtual IMatchMakingProvider* getMatchMaking() const T_OVERRIDE T_FINAL;

	virtual ISaveDataProvider* getSaveData() const T_OVERRIDE T_FINAL;

	virtual IStatisticsProvider* getStatistics() const T_OVERRIDE T_FINAL;

	virtual IUserProvider* getUser() const T_OVERRIDE T_FINAL;

	virtual IVideoSharingProvider* getVideoSharing() const T_OVERRIDE T_FINAL;

	virtual IVoiceChatProvider* getVoiceChat() const T_OVERRIDE T_FINAL;

private:
	Ref< GcAchievements > m_achievements;
	Ref< GcLeaderboards > m_leaderboards;
	Ref< GcMatchMaking > m_matchMaking;
	Ref< GcSaveData > m_saveData;
	Ref< GcStatistics > m_statistics;
	Ref< GcUser > m_user;
	Ref< GcVideoSharing > m_videoSharing;
};

	}
}

#endif	// traktor_online_GcSessionManager_H
