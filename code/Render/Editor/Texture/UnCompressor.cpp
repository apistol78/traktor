/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/UnCompressor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UnCompressor", UnCompressor, ICompressor)

bool UnCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	log::info << L"Writing mip(s)..." << Endl;
	for (RefArray< drawing::Image >::const_iterator i = mipImages.begin(); i != mipImages.end(); ++i)
	{
		int32_t width = (*i)->getWidth();
		int32_t height = (*i)->getHeight();

		uint32_t outputSize = getTextureMipPitch(
			textureFormat,
			width,
			height
		);

		const uint8_t* data = static_cast< const uint8_t* >((*i)->getData());
		if (writer.write(data, outputSize, 1) != outputSize)
			return false;
	}
	log::info << L"All mip(s) written" << Endl;
	return true;
}

	}
}
