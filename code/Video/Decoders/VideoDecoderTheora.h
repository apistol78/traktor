#ifndef traktor_video_VideoDecoderTheora_H
#define traktor_video_VideoDecoderTheora_H

#include "Video/IVideoDecoder.h"

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

class VideoDecoderTheoraImpl;

/*! \brief
 * \ingroup Video
 */
class T_DLLCLASS VideoDecoderTheora : public IVideoDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream);

	virtual void destroy();

	virtual bool getInformation(VideoDecoderInfo& outInfo) const;

	virtual void rewind();

	virtual bool decode(uint32_t frame, void* bits, uint32_t pitch);

private:
	Ref< IStream > m_stream;
	Ref< VideoDecoderTheoraImpl > m_impl;
};

	}
}

#endif	// traktor_video_VideoDecoderTheora_H
