/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class T_DLLCLASS IVoiceChatProvider : public Object
{
	T_RTTI_CLASS;

public:
	struct IVoiceChatCallback
	{
		virtual ~IVoiceChatCallback() {}

		virtual void onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate) = 0;
	};

	virtual ~IVoiceChatProvider() {}

	virtual void setCallback(IVoiceChatCallback* callback) = 0;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) = 0;

	virtual void beginTransmission() = 0;

	virtual void endTransmission() = 0;
};

	}
}

