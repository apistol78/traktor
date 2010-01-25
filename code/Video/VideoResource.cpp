#include "Video/VideoResource.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoResource", 0, VideoResource, ISerializable)

bool VideoResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
