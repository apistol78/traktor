/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/FourCC.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Formats/ImageFormatDds.h"

namespace traktor::drawing
{
	namespace
	{

const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;

#pragma pack(1)

struct DDSPIXELFORMAT
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDSHEADER
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved[11];
	DDSPIXELFORMAT ddspf;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
};

struct DDSHEADER10
{
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlags;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

#pragma pack()

uint32_t packRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ((r << 24) | (g << 16) | (b << 8) | a);
}

void decompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width, const uint8_t *blockStorage, uint32_t *image)
{
	uint16_t color0 = *reinterpret_cast< const uint16_t* >(blockStorage);
	uint16_t color1 = *reinterpret_cast< const uint16_t* >(blockStorage + 2);
 	uint32_t temp;
 
	temp = (color0 >> 11) * 255 + 16;
	uint8_t r0 = (uint8_t)((temp/32 + temp)/32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	uint8_t g0 = (uint8_t)((temp/64 + temp)/64);
	temp = (color0 & 0x001F) * 255 + 16;
	uint8_t b0 = (uint8_t)((temp/32 + temp)/32);
 
	temp = (color1 >> 11) * 255 + 16;
	uint8_t r1 = (uint8_t)((temp/32 + temp)/32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	uint8_t g1 = (uint8_t)((temp/64 + temp)/64);
	temp = (color1 & 0x001F) * 255 + 16;
	uint8_t b1 = (uint8_t)((temp/32 + temp)/32);
 
	uint32_t code = *reinterpret_cast< const uint32_t* >(blockStorage + 4);
 
	for (int32_t j = 0; j < 4; j++)
	{
		for (int32_t i = 0; i < 4; i++)
		{
			uint32_t finalColor = 0;
			uint8_t positionCode = (code >> 2 * (4 * j + i)) & 0x03;
 
			if (color0 > color1)
			{
				switch (positionCode)
				{
					case 0:
						finalColor = packRGBA(r0, g0, b0, 255);
						break;
					case 1:
						finalColor = packRGBA(r1, g1, b1, 255);
						break;
					case 2:
						finalColor = packRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, 255);
						break;
					case 3:
						finalColor = packRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, 255);
						break;
				}
			}
			else
			{
				switch (positionCode)
				{
					case 0:
						finalColor = packRGBA(r0, g0, b0, 255);
						break;
					case 1:
						finalColor = packRGBA(r1, g1, b1, 255);
						break;
					case 2:
						finalColor = packRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, 255);
						break;
					case 3:
						finalColor = packRGBA(0, 0, 0, 255);
						break;
				}
			}
 
			if (x + i < width)
				image[(y + j) * width + (x + i)] = finalColor;
		}
	}
}

void decompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width, const uint8_t* blockStorage, uint32_t* image)
{
	uint8_t alpha0 = *reinterpret_cast< const uint8_t* >(blockStorage);
	uint8_t alpha1 = *reinterpret_cast< const uint8_t* >(blockStorage + 1);
 
	const uint8_t *bits = blockStorage + 2;
	uint32_t alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
	uint16_t alphaCode2 = bits[0] | (bits[1] << 8);
 
	uint16_t color0 = *reinterpret_cast< const uint16_t* >(blockStorage + 8);
	uint16_t color1 = *reinterpret_cast< const uint16_t* >(blockStorage + 10);	
 
	uint32_t temp;
 
	temp = (color0 >> 11) * 255 + 16;
	uint8_t r0 = (uint8_t)((temp/32 + temp)/32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	uint8_t g0 = (uint8_t)((temp/64 + temp)/64);
	temp = (color0 & 0x001F) * 255 + 16;
	uint8_t b0 = (uint8_t)((temp/32 + temp)/32);
 
	temp = (color1 >> 11) * 255 + 16;
	uint8_t r1 = (uint8_t)((temp/32 + temp)/32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	uint8_t g1 = (uint8_t)((temp/64 + temp)/64);
	temp = (color1 & 0x001F) * 255 + 16;
	uint8_t b1 = (uint8_t)((temp/32 + temp)/32);
 
	uint32_t code = *reinterpret_cast< const uint32_t* >(blockStorage + 12);
 
	for (int32_t j = 0; j < 4; j++)
	{
		for (int32_t i = 0; i < 4; i++)
		{
			int32_t alphaCodeIndex = 3 * (4 * j + i);
			int32_t alphaCode;
 
			if (alphaCodeIndex <= 12)
			{
				alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
			}
			else if (alphaCodeIndex == 15)
			{
				alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
			}
			else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
			{
				alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
			}
 
			uint8_t finalAlpha;
			if (alphaCode == 0)
			{
				finalAlpha = alpha0;
			}
			else if (alphaCode == 1)
			{
				finalAlpha = alpha1;
			}
			else
			{
				if (alpha0 > alpha1)
				{
					finalAlpha = ((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7;
				}
				else
				{
					if (alphaCode == 6)
						finalAlpha = 0;
					else if (alphaCode == 7)
						finalAlpha = 255;
					else
						finalAlpha = ((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5;
				}
			}
 
			uint8_t colorCode = (code >> 2*(4*j+i)) & 0x03;
 
			uint32_t finalColor;
			switch (colorCode)
			{
            case 0:
                finalColor = packRGBA(r0, g0, b0, finalAlpha);
                break;
            case 1:
                finalColor = packRGBA(r1, g1, b1, finalAlpha);
                break;
            case 2:
                finalColor = packRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
                break;
            case 3:
                finalColor = packRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
                break;
            default:
                finalColor = 0;
                break;
			}
 
			if (x + i < width)
				image[(y + j)*width + (x + i)] = finalColor;
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatDds", ImageFormatDds, IImageFormat)

Ref< Image > ImageFormatDds::read(IStream* stream)
{
	Reader reader(stream);

	// Read magic.
	uint32_t dwMagic;
	reader >> dwMagic;

	if (dwMagic != 0x20534444)
	{
		log::error << L"Invalid DDS file header." << Endl;
		return nullptr;
	}

	// Read header.
	DDSHEADER ddsh = { 0 };
	reader.read(&ddsh, sizeof(ddsh) / sizeof(uint32_t), sizeof(uint32_t));

	// Convert to pixel format.
	PixelFormat pf;
	if ((ddsh.ddspf.dwFlags & DDPF_RGB) != 0)
	{
		pf = PixelFormat(
			ddsh.ddspf.dwRGBBitCount,
			ddsh.ddspf.dwRBitMask,
			ddsh.ddspf.dwGBitMask,
			ddsh.ddspf.dwBBitMask,
			ddsh.ddspf.dwABitMask,
			false,
			false
		);
	}
	else if ((ddsh.ddspf.dwFlags & DDPF_FOURCC) != 0)
	{
		if (FourCC(ddsh.ddspf.dwFourCC) == FourCC("DXT1"))
			pf = PixelFormat::getR8G8B8A8();
		else if (FourCC(ddsh.ddspf.dwFourCC) == FourCC("DXT5"))
			pf = PixelFormat::getR8G8B8A8();
		else
		{
			log::error << L"Unable to read DDS; \"" << FourCC(ddsh.ddspf.dwFourCC).format() << L"\" not supported." << Endl;
			return nullptr;
		}
	}
	else
		return nullptr;

	// Create image.
	Ref< Image > image = new Image(pf, ddsh.dwWidth, ddsh.dwHeight);

	if (FourCC(ddsh.ddspf.dwFourCC) == FourCC("DXT1"))
	{
		uint32_t blockCountX = (ddsh.dwWidth + 3) / 4;
		uint32_t blockCountY = (ddsh.dwHeight + 3) / 4;
	
		for (uint32_t j = 0; j < blockCountY; j++)
		{
			for (uint32_t i = 0; i < blockCountX; i++)
			{
				uint8_t block[8] = { 0 };
				reader.read(block, 8);
				decompressBlockDXT1(i * 4, j * 4, ddsh.dwWidth, block, (uint32_t*)image->getData());
			}
		}		
	}
	else if (FourCC(ddsh.ddspf.dwFourCC) == FourCC("DXT5"))
	{
		uint32_t blockCountX = (ddsh.dwWidth + 3) / 4;
		uint32_t blockCountY = (ddsh.dwHeight + 3) / 4;
	
		for (uint32_t j = 0; j < blockCountY; j++)
		{
			for (uint32_t i = 0; i < blockCountX; i++)
			{
				uint8_t block[16] = { 0 };
				reader.read(block, 16);
				decompressBlockDXT5(i * 4, j * 4, ddsh.dwWidth, block, (uint32_t*)image->getData());
			}
		}		
	}
	else
	{
		reader.read(
			image->getData(),
			image->getDataSize()
		);
	}
	
	// Append meta data about image.
	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"DDS");
	imageInfo->setGamma(2.2f);
	image->setImageInfo(imageInfo);

	return image;
}

bool ImageFormatDds::write(IStream* stream, const Image* image)
{
	return false;
}

}
