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

void LocalVideoSharing::endCapture()
{
}

bool LocalVideoSharing::showShareUI()
{
	return true;
}

	}
}
