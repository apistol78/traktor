/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

	virtual void destroy() override final;

	virtual bool update() override final;

	virtual std::wstring getLanguageCode() const override final;

	virtual bool isConnected() const override final;

	virtual bool requireFullScreen() const override final;

	virtual bool requireUserAttention() const override final;

	virtual bool haveDLC(const std::wstring& id) const override final;

	virtual bool buyDLC(const std::wstring& id) const override final;

	virtual void overrideDLC(const std::wstring& id, bool set, bool enable) override final;

	virtual bool navigateUrl(const net::Url& url) const override final;

	virtual bool getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const override final;

	virtual bool findFriend(const std::wstring& name, Ref< IUser >& outFriend) const override final;

	virtual bool haveP2PData() const override final;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const override final;

	virtual uint32_t getCurrentGameCount() const override final;

	virtual IAchievements* getAchievements() const override final;

	virtual ILeaderboards* getLeaderboards() const override final;

	virtual IMatchMaking* getMatchMaking() const override final;

	virtual ISaveData* getSaveData() const override final;

	virtual IStatistics* getStatistics() const override final;

	virtual IUser* getUser() const override final;

	virtual IVideoSharing* getVideoSharing() const override final;

	virtual IVoiceChat* getVoiceChat() const override final;

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

