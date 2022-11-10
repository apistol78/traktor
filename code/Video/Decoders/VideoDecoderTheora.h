/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
	virtual bool create(IStream* stream) override final;

	virtual void destroy() override final;

	virtual bool getInformation(VideoDecoderInfo& outInfo) const override final;

	virtual void rewind() override final;

	virtual bool decode(uint32_t frame, void* bits, uint32_t pitch) override final;

private:
	Ref< IStream > m_stream;
	Ref< VideoDecoderTheoraImpl > m_impl;
};

	}
}
