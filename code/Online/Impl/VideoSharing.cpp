/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Impl/VideoSharing.h"
#include "Online/Provider/IVideoSharingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.VideoSharing", VideoSharing, IVideoSharing)

bool VideoSharing::beginCapture(int32_t duration)
{
	if (!m_capturing)
		m_capturing = m_provider->beginCapture(duration);

	return m_capturing;
}

void VideoSharing::endCapture(const PropertyGroup* metaData)
{
	if (m_capturing)
	{
		m_provider->endCapture(metaData);
		m_capturing = false;
	}
}

bool VideoSharing::isCapturing() const
{
	return m_capturing;
}

bool VideoSharing::showShareUI()
{
	return m_provider->showShareUI();
}

VideoSharing::VideoSharing(IVideoSharingProvider* provider)
:	m_provider(provider)
,	m_capturing(false)
{
}

	}
}
