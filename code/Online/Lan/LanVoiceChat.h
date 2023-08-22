/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor::online
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
