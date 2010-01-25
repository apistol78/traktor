#ifndef traktor_video_Video_H
#define traktor_video_Video_H

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

	void update(float deltaTime);

	render::ISimpleTexture* getTexture();

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ISimpleTexture > m_texture;
	float m_time;
	float m_rate;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_video_Video_H
