/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
