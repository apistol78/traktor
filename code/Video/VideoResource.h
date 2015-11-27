#ifndef traktor_video_VideoResource_H
#define traktor_video_VideoResource_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace video
	{

/*! \brief Video resource.
 * \ingroup Video
 */
class T_DLLCLASS VideoResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_video_VideoResource_H
