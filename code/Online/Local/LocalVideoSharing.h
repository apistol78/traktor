/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Provider/IVideoSharingProvider.h"

namespace traktor::online
{

class LocalVideoSharing : public IVideoSharingProvider
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration) override final;

	virtual void endCapture(const PropertyGroup* metaData) override final;

	virtual bool showShareUI() override final;
};

}
