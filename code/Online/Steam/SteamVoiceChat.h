/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <steam/steam_api.h>
#include "Core/Misc/AutoPtr.h"
#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamVoiceChat : public IVoiceChatProvider
{
	T_RTTI_CLASS;

public:
	SteamVoiceChat(SteamSessionManager* sessionManager);

	virtual void setCallback(IVoiceChatCallback* callback) override final;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) override final;

	virtual void beginTransmission() override final;

	virtual void endTransmission() override final;

	void update();

private:
	SteamSessionManager* m_sessionManager;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_voiceBuffer;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_voiceDecompressBuffer;
	IVoiceChatCallback* m_callback;
	bool m_transmitting;
	std::vector< uint64_t > m_audienceHandles;
};

	}
}

