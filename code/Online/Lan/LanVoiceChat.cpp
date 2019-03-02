#include "Online/Lan/LanVoiceChat.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanVoiceChat", LanVoiceChat, IVoiceChatProvider)

void LanVoiceChat::setCallback(IVoiceChatCallback* callback)
{
}

void LanVoiceChat::setAudience(const std::vector< uint64_t >& audienceHandles)
{
}

void LanVoiceChat::beginTransmission()
{
}

void LanVoiceChat::endTransmission()
{
}

	}
}
