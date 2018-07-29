/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_ISessionManager_H
#define traktor_online_ISessionManager_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace net
	{

class Url;

	}

	namespace online
	{

class IAchievements;
class ILeaderboards;
class IMatchMaking;
class ISaveData;
class IStatistics;
class IUser;
class IVideoSharing;
class IVoiceChat;

class T_DLLCLASS ISessionManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool update() = 0;

	virtual std::wstring getLanguageCode() const = 0;

	virtual bool isConnected() const = 0;

	virtual bool requireFullScreen() const = 0;

	virtual bool requireUserAttention() const = 0;

	virtual bool haveDLC(const std::wstring& id) const = 0;

	virtual bool buyDLC(const std::wstring& id) const = 0;

	virtual void overrideDLC(const std::wstring& id, bool set, bool enable) = 0;

	virtual bool navigateUrl(const net::Url& url) const = 0;

	virtual bool getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const = 0;

	virtual bool findFriend(const std::wstring& name, Ref< IUser >& outFriend) const = 0;

	virtual bool haveP2PData() const = 0;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const = 0;

	virtual uint32_t getCurrentGameCount() const = 0;

	virtual IAchievements* getAchievements() const = 0;

	virtual ILeaderboards* getLeaderboards() const = 0;

	virtual IMatchMaking* getMatchMaking() const = 0;

	virtual ISaveData* getSaveData() const = 0;

	virtual IStatistics* getStatistics() const = 0;

	virtual IUser* getUser() const = 0;

	virtual IVideoSharing* getVideoSharing() const = 0;

	virtual IVoiceChat* getVoiceChat() const = 0;
};

	}
}

#endif	// traktor_online_ISessionManager_H
