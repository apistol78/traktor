#pragma once

#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor
{
	namespace online
	{

class LanVoiceChat : public IVoiceChatProvider
{
	T_RTTI_CLASS;

public:
	virtual void setCallback(IVoiceChatCallback* callback) override final;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) override final;

	virtual void beginTransmission() override final;

	virtual void endTransmission() override final;
};

	}
}

