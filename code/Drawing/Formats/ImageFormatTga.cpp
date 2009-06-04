#include "Drawing/Formats/ImageFormatTga.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/New.h"
#include "Core/Io/Reader.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

#pragma pack(1)
struct TGAHEADER
{
	uint8_t identsize;          // size of ID field that follows 18 byte header (0 usually)
	uint8_t colourmaptype;      // type of colour map 0=none, 1=has palette
	uint8_t imagetype;          // type of image 0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed
	uint16_t colourmapstart;    // first colour map entry in palette
	uint16_t colourmaplength;   // number of colours in palette
	uint8_t colourmapbits;      // number of bits per palette entry 15, 16, 24, 32
	uint16_t xstart;            // image x origin
	uint16_t ystart;            // image y origin
	uint16_t width;             // image width in pixels
	uint16_t height;            // image height in pixels
	uint8_t bits;				// image bits per pixel 8,16,24,32
	uint8_t descriptor;			// image descriptor bits (vh flip bits)
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatTga", ImageFormatTga, ImageFormat)

Image* ImageFormatTga::read(Stream* stream)
{
	TGAHEADER header;
	Ref< const PixelFormat > pf;
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

	if (header.imagetype != 2)
		return 0;

	stream->seek(Stream::SeekCurrent, header.identsize);

	switch (header.bits)
	{
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
		pf = gc_new< PixelFormat >(32, 0x00ff0000, 0x0000ff00, 0x000000ff, ((1 << (header.descriptor & 0x0f)) - 1) << 24, false, false);
		break;
	default:
		return 0;
	}

	image = gc_new< Image >(pf, header.width, header.height);

	uint8_t* data = static_cast< uint8_t* >(image->getData());
	stream->read(data, header.width * header.height * pf->getByteSize());

	if (image != 0)
	{
		bool hz = false, ve = true;
		if ((header.descriptor & 0x10) == 0x10)
			hz = true;
		if ((header.descriptor & 0x20) != 0x20)
			ve = true;
		MirrorFilter mirrorFilter(hz, ve);
		image = image->applyFilter(&mirrorFilter);
	}

	Ref< ImageInfo > imageInfo = gc_new< ImageInfo >();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"TGA");
	image->setImageInfo(imageInfo);

	return image;
}

bool ImageFormatTga::write(Stream* stream, Image* image)
{
	TGAHEADER header =
	{
		0,
		0,
		2,
		0,
		0,
		0,
		0,
		0,
		image->getWidth(),
		image->getHeight(),
		image->getPixelFormat()->getColorBits(),
		image->getPixelFormat()->getAlphaBits()
	};

	Ref< Image > clone = image->clone();
	if (!clone)
		return false;

	switch (image->getPixelFormat()->getColorBits())
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
		clone->convert(gc_new< PixelFormat >(32, 0xff000000, 0x00ff0000, 0x0000ff00, (1 << image->getPixelFormat()->getAlphaBits()) - 1, false, false));
		break;
	default:
		return false;
	}

	if (stream->write(&header, sizeof(header)) != sizeof(header))
		return false;

	stream->write(
		clone->getData(),
		clone->getWidth() * clone->getHeight() * clone->getPixelFormat()->getByteSize()
	);
	
	return true;
}

	}
}
