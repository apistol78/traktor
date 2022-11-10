/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Online/Steam/SteamVoiceChat.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_voiceP2PChannel = 1;
const uint32_t c_voiceBufferSize = 32768;
const uint32_t c_voiceDecompressBufferSize = 32768;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamVoiceChat", SteamVoiceChat, IVoiceChatProvider)

SteamVoiceChat::SteamVoiceChat(SteamSessionManager* sessionManager)
:	m_sessionManager(sessionManager)
,	m_voiceBuffer((uint8_t*)Alloc::acquireAlign(c_voiceBufferSize, 16, T_FILE_LINE))
,	m_voiceDecompressBuffer((uint8_t*)Alloc::acquireAlign(c_voiceDecompressBufferSize, 16, T_FILE_LINE))
,	m_callback(nullptr)
,	m_transmitting(false)
{
}

void SteamVoiceChat::setCallback(IVoiceChatCallback* callback)
{
	m_callback = callback;
}

void SteamVoiceChat::setAudience(const std::vector< uint64_t >& audienceHandles)
{
	// Close channels of old audience members.
	for (auto audienceHandle : m_audienceHandles)
	{
		if (std::find(audienceHandles.begin(), audienceHandles.end(), audienceHandle) == audienceHandles.end())
		{
			log::info << L"[Steam Voice] Closing voice channel to " << audienceHandle << L"." << Endl;
			SteamNetworking()->CloseP2PChannelWithUser((uint64)audienceHandle, c_voiceP2PChannel);
		}
	}

	// Replace audience.
	m_audienceHandles = audienceHandles;
}

void SteamVoiceChat::beginTransmission()
{
	if (!m_transmitting)
	{
		SteamUser()->StartVoiceRecording();
		SteamFriends()->SetInGameVoiceSpeaking(SteamUser()->GetSteamID(), true);
		m_transmitting = true;
	}
}

void SteamVoiceChat::endTransmission()
{
	if (m_transmitting)
	{
		SteamUser()->StopVoiceRecording();
		SteamFriends()->SetInGameVoiceSpeaking(SteamUser()->GetSteamID(), false);
		m_transmitting = false;
	}
}

void SteamVoiceChat::update()
{
	// Poll for voice data; transmit to all listening users.
	if (m_transmitting)
	{
		uint32 voiceBufferRead = 0;

		EVoiceResult result = SteamUser()->GetVoice(
			true,
			m_voiceBuffer.ptr(),
			c_voiceBufferSize,
			&voiceBufferRead,
			false,
			0,
			0,
			0,
			11025
		);
		if (result == k_EVoiceResultOK && voiceBufferRead > 0)
		{
			// Transmit recorded data to all listening users.
			for (auto audienceHandle : m_audienceHandles)
			{
				SteamNetworking()->SendP2PPacket(
					(uint64)audienceHandle,
					m_voiceBuffer.c_ptr(),
					voiceBufferRead,
					k_EP2PSendUnreliableNoDelay,
					c_voiceP2PChannel
				);
			}
		}
	}

	// Check if any voice data has been received.
	uint32 voiceDataAvailable = 0;
	if (SteamNetworking()->IsP2PPacketAvailable(&voiceDataAvailable, c_voiceP2PChannel))
	{
		uint32 voiceDataRead = 0;
		CSteamID fromUserID;

		while (SteamNetworking()->ReadP2PPacket(
			m_voiceBuffer.ptr(),
			c_voiceBufferSize,
			&voiceDataRead,
			&fromUserID,
			c_voiceP2PChannel
		))
		{
			uint32 voiceDecodedSize = 0;

			EVoiceResult result = SteamUser()->DecompressVoice(
				m_voiceBuffer.c_ptr(),
				voiceDataRead,
				m_voiceDecompressBuffer.ptr(),
				c_voiceDecompressBufferSize,
				&voiceDecodedSize,
				11025
			);
			if (result == k_EVoiceResultOK)
			{
				if (m_callback)
					m_callback->onVoiceReceived(
						fromUserID.ConvertToUint64(),
						(const int16_t*)m_voiceDecompressBuffer.c_ptr(),
						voiceDecodedSize / sizeof(int16_t),
						11025
					);
			}
		}
	}
}

	}
}
