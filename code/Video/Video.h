#ifndef traktor_video_Video_H
#define traktor_video_Video_H

#include "Core/Object.h"
#include "Video/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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

	bool create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder);

	void destroy();

	bool update(float deltaTime);

	bool playing() const;

	void rewind();

	render::ISimpleTexture* getTexture();

	VideoFormat getFormat() const;

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ISimpleTexture > m_textures[2];
	VideoFormat m_format;
	float m_time;
	float m_rate;
	uint32_t m_frame;
	uint32_t m_current;
	bool m_playing;
};

	}
}

#endif	// traktor_video_Video_H
