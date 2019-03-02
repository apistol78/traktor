#pragma once

#include "Online/Gc/GcVideoSharing.h"

namespace traktor
{
	namespace online
	{

class GcGameConfiguration;

class GcVideoSharingEveryplay : public GcVideoSharing
{
	T_RTTI_CLASS;

public:
	bool create(const GcGameConfiguration& configuration);

	virtual bool beginCapture(int32_t duration) T_OVERRIDE T_FINAL;

	virtual void endCapture(const PropertyGroup* metaData) T_OVERRIDE T_FINAL;

	virtual bool showShareUI() T_OVERRIDE T_FINAL;
};

	}
}

