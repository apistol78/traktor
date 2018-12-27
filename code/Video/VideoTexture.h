/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

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

	virtual ~VideoTexture();

	bool create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder);

	virtual void destroy() override final;

	virtual render::ITexture* resolve() override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool lock(int level, render::ITexture::Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

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
