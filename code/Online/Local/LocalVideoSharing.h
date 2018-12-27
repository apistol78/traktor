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
	virtual bool beginCapture(int32_t duration) override final;

	virtual void endCapture(const PropertyGroup* metaData) override final;

	virtual bool showShareUI() override final;
};

	}
}

#endif	// traktor_online_LocalVideoSharing_H
