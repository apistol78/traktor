#ifndef traktor_online_GcVideoSharingKamcord_H
#define traktor_online_GcVideoSharingKamcord_H

#include "Online/Gc/GcVideoSharing.h"

namespace traktor
{
	namespace online
	{

class GcVideoSharingKamcord : public GcVideoSharing
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration);

	virtual void endCapture();

	virtual bool showShareUI();
};

	}
}

#endif	// traktor_online_GcVideoSharingKamcord_H
