#include "Video/VideoResource.h"
#include "Video/Editor/VideoAsset.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.video.VideoAsset", 0, VideoAsset, editor::Asset)

const TypeInfo* VideoAsset::getOutputType() const
{
	return &type_of< VideoResource >();
}

	}
}
