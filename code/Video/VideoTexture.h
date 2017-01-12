#ifndef traktor_video_VideoTexture_H
#define traktor_video_VideoTexture_H

#include "Core/Misc/AutoPtr.h"
#include "Core/Timer/Timer.h"
#include "Render/ISimpleTexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace render
	{

class IRenderSystem;

	}

	namespace video
	{
	
class IVideoDecoder;

class T_DLLCLASS VideoTexture : public render::ISimpleTexture
{
	T_RTTI_CLASS;

public:
	VideoTexture();

	bool create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual render::ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, render::ITexture::Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ISimpleTexture > m_textures[4];
	Timer m_timer;
	float m_rate;
	AutoPtr< uint8_t, AllocFreeAlign > m_frameBuffer;
	uint32_t m_frameBufferPitch;
	uint32_t m_frameBufferSize;
	uint32_t m_lastDecodedFrame;
	uint32_t m_lastUploadedFrame;
	uint32_t m_current;
	Thread* m_thread;

	void decodeThread();
};

	}
}

#endif	// traktor_video_VideoTexture_H
