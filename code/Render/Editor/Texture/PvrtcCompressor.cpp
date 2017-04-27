/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"

#if defined(T_HAVE_PVRTC)
#	include "pvrtc_dll.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PvrtcCompressor", PvrtcCompressor, ICompressor)

bool PvrtcCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
#if defined(T_HAVE_PVRTC)
	log::info << L"Compressing and writing mip(s)..." << Endl;

	int32_t mipCount = int32_t(mipImages.size());
	int32_t use2Bit = (textureFormat == TfPVRTC2 || textureFormat == TfPVRTC4) ? 1 : 0;

	int32_t maxByteSize = pvrtc_size(mipImages[0]->getWidth(), mipImages[0]->getHeight(), 0, use2Bit);
	AutoPtr< uint8_t > compressedData(new uint8_t [maxByteSize]);

	for (int32_t i = 0; i < mipCount; ++i)
	{
		int32_t byteSize = pvrtc_size(mipImages[i]->getWidth(), mipImages[i]->getHeight(), 0, use2Bit);
		T_ASSERT (byteSize <= maxByteSize);

		Ref< drawing::Image > mipImage = mipImages[i];
		mipImage->convert(drawing::PixelFormat::getA8R8G8B8());

		pvrtc_compress(
			mipImage->getData(),
			compressedData.ptr(),
			mipImage->getWidth(),
			mipImage->getHeight(),
			0,
			needAlpha ? 1 : 0,
			1,
			use2Bit
		);

		if (writer.write(compressedData.ptr(), byteSize, 1) != byteSize)
			return false;
	}

	log::info << L"All mip(s) written" << Endl;
	return true;
#else
    return false;
#endif
}

	}
}
