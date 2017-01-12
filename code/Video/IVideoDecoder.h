#ifndef traktor_video_IVideoDecoder_H
#define traktor_video_IVideoDecoder_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace video
	{

/*! \brief Video decoder information.
 * \ingroup Video
 */
struct VideoDecoderInfo
{
	uint32_t width;
	uint32_t height;
	float rate;
};

/*! \brief Video decoder stream interface.
 * \ingroup Video
 */
class T_DLLCLASS IVideoDecoder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) = 0;

	virtual void destroy() = 0;

	virtual bool getInformation(VideoDecoderInfo& outInfo) const = 0;

	virtual void rewind() = 0;

	virtual bool decode(uint32_t frame, void* bits, uint32_t pitch) = 0;
};

	}
}

#endif	// traktor_video_IVideoDecoder_H
