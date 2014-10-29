#ifndef traktor_online_GcVideoSharingEveryplay_H
#define traktor_online_GcVideoSharingEveryplay_H

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

	virtual bool beginCapture(int32_t duration);

	virtual void endCapture();

	virtual bool showShareUI();
};

	}
}

#endif	// traktor_online_GcVideoSharingEveryplay_H
