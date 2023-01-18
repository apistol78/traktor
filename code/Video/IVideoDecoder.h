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

}

namespace traktor::video
{

/*! Video decoder information.
 * \ingroup Video
 */
struct VideoDecoderInfo
{
	uint32_t width;
	uint32_t height;
	float rate;
};

/*! Video decoder stream interface.
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
