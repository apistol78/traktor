/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual void setCallback(IVoiceChatCallback* callback) override final;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) override final;

	virtual void beginTransmission() override final;

	virtual void endTransmission() override final;
};

	}
}

#endif	// traktor_online_LanVoiceChat_H
