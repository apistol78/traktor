#ifndef traktor_online_LanVoiceChat_H
#define traktor_online_LanVoiceChat_H

#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor
{
	namespace online
	{

class LanVoiceChat : public IVoiceChatProvider
{
	T_RTTI_CLASS;

public:
	virtual void setCallback(IVoiceChatCallback* callback);

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles);

	virtual void beginTransmission();

	virtual void endTransmission();
};

	}
}

#endif	// traktor_online_LanVoiceChat_H
