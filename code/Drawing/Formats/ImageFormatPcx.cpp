/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/Formats/ImageFormatPcx.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Palette.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatPcx", ImageFormatPcx, IImageFormat)

#pragma pack(1)

namespace
{

struct PCXHEADER
{
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bitsPerPixel;
	uint16_t xmin;
	uint16_t ymin;
	uint16_t xmax;
	uint16_t ymax;
	uint16_t vdpi;
	uint8_t palette[48];
	uint8_t reserved;
	uint8_t planes;
	uint16_t pitch;
	uint16_t paletteType;
	uint16_t screenWidth;
	uint16_t screenHeight;
	uint8_t dummy[56];
};

}

#pragma pack()

Ref< Image > ImageFormatPcx::read(IStream* stream)
{
	PCXHEADER hdr;
	Reader reader(stream);

	reader >> hdr.manufacturer;
	reader >> hdr.version;
	reader >> hdr.encoding;
	reader >> hdr.bitsPerPixel;
	reader >> hdr.xmin;
	reader >> hdr.ymin;
	reader >> hdr.xmax;
	reader >> hdr.ymax;
	reader >> hdr.vdpi;
	reader.read(hdr.palette, 48, sizeof(uint8_t));
	reader >> hdr.reserved;
	reader >> hdr.planes;
	reader >> hdr.pitch;
	reader >> hdr.paletteType;
	reader >> hdr.screenWidth;
	reader >> hdr.screenHeight;
	reader.read(hdr.dummy, 56, sizeof(uint8_t));

	if (hdr.bitsPerPixel != 8 || hdr.planes != 1)
	{
		log::error << L"Unsupported PCX file, must be 8 bits per pixel and a single plane" << Endl;
		return 0;
	}

	Ref< Palette > palette = new Palette();
	Ref< Image > image = new Image(PixelFormat::getP8(), hdr.xmax - hdr.xmin, hdr.ymax - hdr.ymin, palette);

	uint8_t T_UNALIGNED *bits = static_cast< uint8_t* >(image->getData());

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		uint8_t T_UNALIGNED *scan = &bits[y * image->getWidth()];

		int count = 0;
		uint8_t value = 0;

		int32_t x = image->getWidth();
		while (x > 0)
		{
			uint8_t c;
			if (stream->read(&c, sizeof(uint8_t)) != sizeof(uint8_t))
			{
				log::error << L"Unexpected end of stream" << Endl;
				return 0;
			}
			if ((c & 0xc0) == 0xc0)
			{
				count = c & 0x3f;
				if (stream->read(&value, sizeof(uint8_t)) != sizeof(uint8_t))
				{
					log::error << L"Unexpected end of stream" << Endl;
					return 0;
				}
			}
			else
			{
				count = 1;
				value = c;
			}

			if (count > x)
				count = x;

			x -= count;

			while (count-- > 0)
				*scan++ = value;
		}
	}

	uint8_t dummy;
	stream->read(&dummy, sizeof(dummy));
	T_ASSERT (dummy == 12);

	for (uint32_t i = 0; i < 256; ++i)
	{
		uint8_t rgb[3];
		stream->read(rgb, 3);
		palette->set(i, Color4f(
			rgb[0] / 255.0f,
			rgb[1] / 255.0f,
			rgb[2] / 255.0f
		));
	}

	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"PCX");
	image->setImageInfo(imageInfo);

	return image;
}

bool ImageFormatPcx::write(IStream* stream, Image* image)
{
	PCXHEADER hdr;
	Writer writer(stream);

	if (!image->getPixelFormat().isPalettized())
		return false;

	hdr.manufacturer = 0x0a;
	hdr.version = 5;
	hdr.encoding = 1;
	hdr.bitsPerPixel = 8;
	hdr.xmin = 0;
	hdr.ymin = 0;
	hdr.xmax = uint16_t(image->getWidth());
	hdr.ymax = uint16_t(image->getHeight());
	hdr.vdpi = 100;
	hdr.reserved = 0;
	hdr.planes = 1;
	hdr.pitch = uint16_t(image->getWidth());
	hdr.paletteType = 0;
	hdr.screenWidth = 0;
	hdr.screenHeight = 0;
	hdr.dummy[55] = 0xff;

	writer << hdr.manufacturer;
	writer << hdr.version;
	writer << hdr.encoding;
	writer << hdr.bitsPerPixel;
	writer << hdr.xmin;
	writer << hdr.ymin;
	writer << hdr.xmax;
	writer << hdr.ymax;
	writer << hdr.vdpi;
	writer.write(hdr.palette, 48, sizeof(uint8_t));
	writer << hdr.reserved;
	writer << hdr.planes;
	writer << hdr.pitch;
	writer << hdr.paletteType;
	writer << hdr.screenWidth;
	writer << hdr.screenHeight;
	writer.write(hdr.dummy, 56, sizeof(uint8_t));

	const uint8_t* bits = static_cast< const uint8_t* >(image->getData());

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		const uint8_t* scan = &bits[y * image->getWidth()];

		int32_t x = 0;
		while (x < image->getWidth())
		{
			uint8_t value = *scan++;
			uint8_t count = 1;

			while (*scan == value && count < 0x3f && x + count < image->getWidth())
			{
				count++;
				scan++;
			}

			if (count > 1 || value > 0x3f)
			{
				writer << uint8_t(count | 0xc0);
				writer << uint8_t(value);
			}
			else
			{
				writer << value;
			}

			x += count;
		}
	}

	const Palette* palette = image->getPalette();
	if (!palette)
		return false;

	writer << uint8_t(12);

	for (int i = 0; i < palette->getSize(); ++i)
	{
		const Color4f& c = palette->get(i);
		writer << uint8_t(c.getRed() * 255.0f);
		writer << uint8_t(c.getGreen() * 255.0f);
		writer << uint8_t(c.getBlue() * 255.0f);
	}
	for (int i = palette->getSize(); i < 256; ++i)
	{
		writer << uint8_t(255);
		writer << uint8_t(255);
		writer << uint8_t(255);
	}

	return true;
}

	}
}
