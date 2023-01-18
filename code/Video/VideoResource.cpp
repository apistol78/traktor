/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Video/VideoResource.h"

namespace traktor::video
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoResource", 0, VideoResource, ISerializable)

void VideoResource::serialize(ISerializer& s)
{
}

}
