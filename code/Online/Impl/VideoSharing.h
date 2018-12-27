/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_VideoSharing_H
#define traktor_online_VideoSharing_H

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

#endif	// traktor_online_VideoSharing_H
