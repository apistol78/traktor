#ifndef traktor_online_SteamVoiceChat_H
#define traktor_online_SteamVoiceChat_H

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

	virtual void setCallback(IVoiceChatCallback* callback);

	virtual void beginTransmission(const std::vector< uint64_t >& audienceHandles);

	virtual void endTransmission();

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

#endif	// traktor_online_SteamVoiceChat_H
