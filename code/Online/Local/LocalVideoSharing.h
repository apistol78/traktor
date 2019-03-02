#pragma once

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

