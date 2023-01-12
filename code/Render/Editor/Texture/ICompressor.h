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
#include "Core/RefArray.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Writer;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::render
{

/*! Texture compressor interface.
 * \ingroup Render
 */
class T_DLLCLASS ICompressor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const = 0;
};

}
