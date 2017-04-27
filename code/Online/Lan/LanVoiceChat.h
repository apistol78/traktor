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
	virtual void setCallback(IVoiceChatCallback* callback) T_OVERRIDE T_FINAL;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) T_OVERRIDE T_FINAL;

	virtual void beginTransmission() T_OVERRIDE T_FINAL;

	virtual void endTransmission() T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_LanVoiceChat_H
