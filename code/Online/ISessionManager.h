/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
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

namespace traktor::net
{

class Url;

}

namespace traktor::online
{

class IAchievements;
class ILeaderboards;
class IMatchMaking;
class ISaveData;
class IStatistics;
class IUser;
class IVideoSharing;
class IVoiceChat;

/*! Session manager interface.
 * \ingroup Online
 */
class T_DLLCLASS ISessionManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool update() = 0;

	/*! Get language code. */
	virtual std::wstring getLanguageCode() const = 0;

	/*! Check if connected to service provider. */
	virtual bool isConnected() const = 0;

	/*! Check if service require application in fullscreen mode. */
	virtual bool requireFullScreen() const = 0;

	/*! Check if service require user attention. */
	virtual bool requireUserAttention() const = 0;

	/*! Check if signed in user own DLC entitlement. */
	virtual bool haveDLC(const std::wstring& id) const = 0;

	/*! Try to buy DLC entitlement. */
	virtual bool buyDLC(const std::wstring& id) const = 0;

	/*! Override DLC entitlement ownership.
	 * \note
	 * This is only temporary and will get reset
	 * after application terminate.
	 */
	virtual void overrideDLC(const std::wstring& id, bool set, bool enable) = 0;

	/*! Open URL in service designated web browser. */
	virtual bool navigateUrl(const net::Url& url) const = 0;

	/*! Get friends if signed in user. */
	virtual bool getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const = 0;

	/*! Find specific friend of signed in user. */
	virtual bool findFriend(const std::wstring& name, Ref< IUser >& outFriend) const = 0;

	/*! Check if pending P2P data queued. */
	virtual bool haveP2PData() const = 0;

	/*! Receive P2P data sent from another user. */
	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const = 0;

	/*! Get number of concurrent game instances live on service. */
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
