/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LocalVideoSharing_H
#define traktor_online_LocalVideoSharing_H

#include "Online/Provider/IVideoSharingProvider.h"

namespace traktor
{
	namespace online
	{

class LocalVideoSharing : public IVideoSharingProvider
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration) T_OVERRIDE T_FINAL;

	virtual void endCapture(const PropertyGroup* metaData) T_OVERRIDE T_FINAL;

	virtual bool showShareUI() T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_LocalVideoSharing_H
