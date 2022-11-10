/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Local/LocalVideoSharing.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalVideoSharing", LocalVideoSharing, IVideoSharingProvider)

bool LocalVideoSharing::beginCapture(int32_t duration)
{
	return true;
}

void LocalVideoSharing::endCapture(const PropertyGroup* metaData)
{
}

bool LocalVideoSharing::showShareUI()
{
	return true;
}

	}
}
