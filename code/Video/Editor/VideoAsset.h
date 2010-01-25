#ifndef traktor_video_VideoAsset_H
#define traktor_video_VideoAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace video
	{

/*! \brief
 * \ingroup Video
 */
class T_DLLCLASS VideoAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo* getOutputType() const;
};

	}
}

#endif	// traktor_video_VideoAsset_H
