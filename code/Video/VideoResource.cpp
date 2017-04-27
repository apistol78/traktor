/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Video/VideoResource.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoResource", 0, VideoResource, ISerializable)

void VideoResource::serialize(ISerializer& s)
{
}

	}
}
