#ifndef traktor_video_Video_H
#define traktor_video_Video_H

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace video
	{

class IVideoDecoder;

/*! \brief
 * \ingroup Video
 */
class T_DLLCLASS Video : public Object
{
	T_RTTI_CLASS;

public:
	Video();

	virtual ~Video();

	bool create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder);

	void destroy();

	bool update(float deltaTime);

	bool playing() const;

	void rewind();

	render::ISimpleTexture* getTexture();

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ISimpleTexture > m_textures[4];
	float m_time;
	float m_rate;
	AutoPtr< uint8_t, AllocFreeAlign > m_frameBuffer;
	uint32_t m_frameBufferPitch;
	uint32_t m_frameBufferSize;
	bool m_playing;
	uint32_t m_lastDecodedFrame;
	uint32_t m_lastUploadedFrame;
	uint32_t m_current;
	Ref< Job > m_job;

	void decodeFrame(uint32_t frame);
};

	}
}

#endif	// traktor_video_Video_H
