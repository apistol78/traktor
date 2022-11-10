/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Online/IVideoSharing.h"

namespace traktor
{
	namespace online
	{

class IVideoSharingProvider;

class VideoSharing : public IVideoSharing
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration) override final;

	virtual void endCapture(const PropertyGroup* metaData) override final;

	virtual bool isCapturing() const override final;

	virtual bool showShareUI() override final;

private:
	friend class SessionManager;

	Ref< IVideoSharingProvider > m_provider;
	bool m_capturing;

	VideoSharing(IVideoSharingProvider* provider);
};

	}
}

