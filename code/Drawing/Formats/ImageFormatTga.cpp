/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Formats/ImageFormatTga.h"

#include <cstring>
#include "Core/Io/Reader.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{
namespace
{

#pragma pack(1)

struct TGAHEADER
{
	uint8_t identsize;		  // size of ID field that follows 18 byte header (0 usually)
	uint8_t colourmaptype;	  // type of colour map 0=none, 1=has palette
	uint8_t imagetype;		  // type of image 0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed
	uint16_t colourmapstart;  // first colour map entry in palette
	uint16_t colourmaplength; // number of colours in palette
	uint8_t colourmapbits;	  // number of bits per palette entry 15, 16, 24, 32
	uint16_t xstart;		  // image x origin
	uint16_t ystart;		  // image y origin
	uint16_t width;			  // image width in pixels
	uint16_t height;		  // image height in pixels
	uint8_t bits;			  // image bits per pixel 8,16,24,32
	uint8_t descriptor;		  // image descriptor bits (vh flip bits)
};

#pragma pack()

const char c_footerSignature[] = "TRUEVISION-XFILE";

const uint32_t c_footerSize = 26;				//!< Size of TGA 2.0 file footer.
const uint32_t c_footerSignatureOffset = 8;		//!< Offset of signature within footer.
const uint32_t c_extensionSize = 495;			//!< Size of TGA 2.0 extension area.
const uint32_t c_extensionSoftwareOffset = 426;	//!< Offset of software ID within extension area.
const uint32_t c_extensionGammaOffset = 478;	//!< Offset of gamma value within extension area.
const uint32_t c_extensionAttributesOffset = 494;	//!< Offset of attributes type within extension area.

/*! Default gamma when image doesn't specify one; assume sRGB as most TGAs are authored as such. */
const float c_defaultGamma = 2.2f;

uint16_t readU16(const uint8_t* p)
{
	return (uint16_t)(p[0] | (p[1] << 8));
}

uint32_t readU32(const uint8_t* p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

void writeU16(uint8_t* p, uint16_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
}

void writeU32(uint8_t* p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatTga", ImageFormatTga, IImageFormat)

Ref< Image > ImageFormatTga::read(IStream* stream)
{
	TGAHEADER header;
	PixelFormat pf;
	Ref< Image > image;

	Reader reader(stream);
	reader >> header.identsize;
	reader >> header.colourmaptype;
	reader >> header.imagetype;
	reader >> header.colourmapstart;
	reader >> header.colourmaplength;
	reader >> header.colourmapbits;
	reader >> header.xstart;
	reader >> header.ystart;
	reader >> header.width;
	reader >> header.height;
	reader >> header.bits;
	reader >> header.descriptor;

	if (header.imagetype != 2 && header.imagetype != 3)
		return nullptr;

	stream->seek(IStream::SeekCurrent, header.identsize);

	switch (header.bits)
	{
	case 8:
		pf = PixelFormat(8, 0xff, 0xff, 0xff, 0xff, false, false);
		break;
	case 15:
		pf = PixelFormat::getR5G5B5();
		break;
	case 16:
		pf = PixelFormat::getR5G6B5();
		break;
	case 24:
		pf = PixelFormat::getR8G8B8();
		break;
	case 32:
		pf = PixelFormat(32, 0x00ff0000, 0x0000ff00, 0x000000ff, ((1 << (header.descriptor & 0x0f)) - 1) << 24, false, false);
		break;
	default:
		return nullptr;
	}

	image = new Image(pf, header.width, header.height);

	if (header.width > 0 && header.height > 0)
	{
		uint8_t* data = static_cast< uint8_t* >(image->getData());
		stream->read(data, header.width * header.height * pf.getByteSize());

		if (image != 0)
		{
			bool hz = false, ve = true;
			if ((header.descriptor & 0x10) == 0x10)
				hz = true;
			if ((header.descriptor & 0x20) == 0x20)
				ve = false;
			if (hz || ve)
			{
				const MirrorFilter mirrorFilter(hz, ve);
				image->apply(&mirrorFilter);
			}
		}
	}

	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"TGA");
	imageInfo->setGamma(c_defaultGamma);

	// Gamma is stored in the TGA 2.0 extension area, which is located through
	// the offset in the file footer. Both are optional; TGA 1.0 files have neither.
	if (stream->canSeek())
	{
		uint8_t footer[c_footerSize];
		stream->seek(IStream::SeekEnd, -(int64_t)c_footerSize);
		if (
			stream->read(footer, c_footerSize) == c_footerSize &&
			std::memcmp(&footer[c_footerSignatureOffset], c_footerSignature, sizeof(c_footerSignature) - 1) == 0)
		{
			const uint32_t extensionOffset = readU32(&footer[0]);
			if (extensionOffset != 0)
			{
				uint8_t extension[c_extensionSize];
				stream->seek(IStream::SeekSet, extensionOffset);
				if (
					stream->read(extension, c_extensionSize) == c_extensionSize &&
					readU16(&extension[0]) == c_extensionSize)
				{
					// Gamma is stored as a numerator/denominator pair, ex 22/10 for gamma 2.2.
					const uint16_t gammaNumerator = readU16(&extension[c_extensionGammaOffset]);
					const uint16_t gammaDenom = readU16(&extension[c_extensionGammaOffset + 2]);
					if (gammaNumerator != 0 && gammaDenom != 0)
						imageInfo->setGamma(clamp(float(gammaNumerator) / gammaDenom, 0.0f, 10.0f));
				}
			}
		}
	}

	image->setImageInfo(imageInfo);

	return image;
}

bool ImageFormatTga::write(IStream* stream, const Image* image)
{
	TGAHEADER header;
	header.identsize = 0;
	header.colourmaptype = 0;
	header.imagetype = 2;
	header.colourmapstart = 0;
	header.colourmaplength = 0;
	header.colourmapbits = 0;
	header.xstart = 0;
	header.ystart = 0;
	header.width = image->getWidth();
	header.height = image->getHeight();
	header.bits = image->getPixelFormat().getColorBits();
	header.descriptor = 0x20 | image->getPixelFormat().getAlphaBits();

	Ref< Image > clone = image->clone();
	if (!clone)
		return false;

	switch (image->getPixelFormat().getColorBits())
	{
	case 15:
		clone->convert(PixelFormat::getR5G5B5());
		break;
	case 16:
		clone->convert(PixelFormat::getR5G6B5());
		break;
	case 24:
		clone->convert(PixelFormat::getR8G8B8());
		break;
	case 32:
		{
			PixelFormat pixelFormat(32, 0x00ff0000, 0x0000ff00, 0x000000ff, ((1 << image->getPixelFormat().getAlphaBits()) - 1) << 24, false, false);
			clone->convert(pixelFormat);
		}
		break;
	default:
		return false;
	}

	if (stream->write(&header, sizeof(header)) != sizeof(header))
		return false;

	const int64_t imageDataSize = clone->getWidth() * clone->getHeight() * clone->getPixelFormat().getByteSize();
	if (stream->write(clone->getData(), imageDataSize) != imageDataSize)
		return false;

	// Write TGA 2.0 extension area, carrying the gamma value, followed by the file footer
	// which locates the extension area.
	const int64_t extensionOffset = stream->tell();

	uint8_t extension[c_extensionSize] = { 0 };
	writeU16(&extension[0], c_extensionSize);
	std::memcpy(&extension[c_extensionSoftwareOffset], "Traktor", 7);

	// Gamma is stored as a numerator/denominator pair, ex 22/10 for gamma 2.2; left
	// zeroed, ie unspecified, if the image doesn't carry a gamma value.
	const ImageInfo* imageInfo = image->getImageInfo();
	if (imageInfo != nullptr && imageInfo->getGamma() > 0.0f)
	{
		writeU16(&extension[c_extensionGammaOffset], (uint16_t)(clamp(imageInfo->getGamma(), 0.0f, 10.0f) * 10.0f + 0.5f));
		writeU16(&extension[c_extensionGammaOffset + 2], 10);
	}

	// Attributes type; 3 = useful alpha channel data, 0 = no alpha data included.
	extension[c_extensionAttributesOffset] = (image->getPixelFormat().getAlphaBits() > 0) ? 3 : 0;

	if (stream->write(extension, c_extensionSize) != c_extensionSize)
		return false;

	uint8_t footer[c_footerSize] = { 0 };
	writeU32(&footer[0], (uint32_t)extensionOffset);
	writeU32(&footer[4], 0);	// No developer directory.
	std::memcpy(&footer[c_footerSignatureOffset], c_footerSignature, sizeof(c_footerSignature) - 1);
	footer[24] = '.';			// Reserved character, must be a period.

	if (stream->write(footer, c_footerSize) != c_footerSize)
		return false;

	return true;
}

}
