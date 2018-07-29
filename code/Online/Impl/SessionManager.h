/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_SessionManager_H
#define traktor_online_SessionManager_H

#include "Core/RefArray.h"
#include "Online/ISessionManager.h"

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

class Achievements;
class IGameConfiguration;
class ISessionManagerProvider;
class IUser;
class Leaderboards;
class MatchMaking;
class SaveData;
class Statistics;
class TaskQueue;
class TaskUpdateSessionManager;
class UserCache;
class VideoSharing;
class VoiceChat;

class T_DLLCLASS SessionManager : public ISessionManager
{
	T_RTTI_CLASS;

public:
	SessionManager();

	bool create(
		ISessionManagerProvider* provider,
		const IGameConfiguration* configuration,
		bool downloadableContent,
		const std::wstring& overrideLanguageCode
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool update() T_OVERRIDE T_FINAL;

	virtual std::wstring getLanguageCode() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual bool requireFullScreen() const T_OVERRIDE T_FINAL;

	virtual bool requireUserAttention() const T_OVERRIDE T_FINAL;

	virtual bool haveDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool buyDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual void overrideDLC(const std::wstring& id, bool set, bool enable) T_OVERRIDE T_FINAL;

	virtual bool navigateUrl(const net::Url& url) const T_OVERRIDE T_FINAL;

	virtual bool getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const T_OVERRIDE T_FINAL;

	virtual bool findFriend(const std::wstring& name, Ref< IUser >& outFriend) const T_OVERRIDE T_FINAL;

	virtual bool haveP2PData() const T_OVERRIDE T_FINAL;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const T_OVERRIDE T_FINAL;

	virtual uint32_t getCurrentGameCount() const T_OVERRIDE T_FINAL;

	virtual IAchievements* getAchievements() const T_OVERRIDE T_FINAL;

	virtual ILeaderboards* getLeaderboards() const T_OVERRIDE T_FINAL;

	virtual IMatchMaking* getMatchMaking() const T_OVERRIDE T_FINAL;

	virtual ISaveData* getSaveData() const T_OVERRIDE T_FINAL;

	virtual IStatistics* getStatistics() const T_OVERRIDE T_FINAL;

	virtual IUser* getUser() const T_OVERRIDE T_FINAL;

	virtual IVideoSharing* getVideoSharing() const T_OVERRIDE T_FINAL;

	virtual IVoiceChat* getVoiceChat() const T_OVERRIDE T_FINAL;

private:
	Ref< ISessionManagerProvider > m_provider;
	Ref< TaskQueue > m_taskQueues[2];
	Ref< Achievements > m_achievements;
	Ref< Leaderboards > m_leaderboards;
	Ref< MatchMaking > m_matchMaking;
	Ref< SaveData > m_saveData;
	Ref< Statistics > m_statistics;
	Ref< IUser > m_user;
	Ref< VideoSharing > m_videoSharing;
	Ref< VoiceChat > m_voiceChat;
	Ref< UserCache > m_userCache;
	Ref< TaskUpdateSessionManager > m_updateTask;
	std::wstring m_overrideLanguageCode;
	bool m_downloadableContent;
	std::map< std::wstring, bool > m_overrideDLC;
	bool m_connected;
};

	}
}

#endif	// traktor_online_SessionManager_H
