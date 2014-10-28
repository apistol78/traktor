#include "Online/Gc/GcVideoSharingKamcord.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcVideoSharingKamcord", GcVideoSharingKamcord, GcVideoSharing)

bool GcVideoSharingKamcord::beginCapture(int32_t duration)
{
	return false;
}

void GcVideoSharingKamcord::endCapture()
{
}

bool GcVideoSharingKamcord::showShareUI()
{
	return false;
}

	}
}
