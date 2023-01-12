/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Writer.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/UnCompressor.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UnCompressor", UnCompressor, ICompressor)

bool UnCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	for (RefArray< drawing::Image >::const_iterator i = mipImages.begin(); i != mipImages.end(); ++i)
	{
		const int32_t width = (*i)->getWidth();
		const int32_t height = (*i)->getHeight();

		const uint32_t outputSize = getTextureMipPitch(
			textureFormat,
			width,
			height
		);

		const uint8_t* data = static_cast< const uint8_t* >((*i)->getData());
		if (writer.write(data, outputSize, 1) != outputSize)
			return false;
	}
	return true;
}

}
