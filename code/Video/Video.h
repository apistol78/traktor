/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

}

namespace traktor::render
{

class IRenderSystem;
class ITexture;

}

namespace traktor::video
{

class IVideoDecoder;

/*!
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

	render::ITexture* getTexture();

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ITexture > m_textures[4];
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
