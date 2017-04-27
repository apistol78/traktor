/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
,	m_callback(0)
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
	for (std::vector< uint64_t >::const_iterator i = m_audienceHandles.begin(); i != m_audienceHandles.end(); ++i)
	{
		if (std::find(audienceHandles.begin(), audienceHandles.end(), *i) == audienceHandles.end())
		{
			log::info << L"[Steam Voice] Closing voice channel to " << *i << Endl;
			SteamNetworking()->CloseP2PChannelWithUser(uint64(*i), c_voiceP2PChannel);
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
			T_DEBUG(L"Sending voice to " << int32_t(m_audienceHandles.size()) << L" user(s)");

			// Transmit recorded data to all listening users.
			for (std::vector< uint64_t >::const_iterator i = m_audienceHandles.begin(); i != m_audienceHandles.end(); ++i)
			{
				SteamNetworking()->SendP2PPacket(
					uint64(*i),
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
