/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Online/IVoiceChat.h"
#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor
{
	namespace sound
	{

class ISoundHandle;

	}

	namespace online
	{

class UserCache;

class VoiceChat
:	public IVoiceChat
,	public IVoiceChatProvider::IVoiceChatCallback
{
	T_RTTI_CLASS;

public:
	void destroy();

	virtual void setSoundPlayer(sound::ISoundPlayer* soundPlayer) override final;

	virtual void setAudience(const RefArray< IUser >& audience) override final;

	virtual void beginTransmission() override final;

	virtual void endTransmission() override final;

	virtual void setMute(bool mute) override final;

	virtual bool isTransmitting(IUser* user) override final;

private:
	friend class SessionManager;

	struct Transmission
	{
		Ref< IUser > user;
		Ref< class VoiceSoundBuffer > soundBuffer;
		Ref< sound::ISoundHandle > soundHandle;
	};

	Semaphore m_lock;
	Ref< IVoiceChatProvider > m_provider;
	Ref< UserCache > m_userCache;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	uint32_t m_category;
	bool m_muted;
	std::list< Transmission > m_transmissions;

	VoiceChat(IVoiceChatProvider* provider, UserCache* userCache);

	virtual ~VoiceChat();

	virtual void onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate) override final;
};

	}
}

