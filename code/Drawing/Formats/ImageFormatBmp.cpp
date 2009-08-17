#include "Drawing/Formats/ImageFormatBmp.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/GcNew.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatBmp", ImageFormatBmp, ImageFormat)

#pragma pack(1)

namespace
{

struct BITMAPFILEHEADER
{
	uint8_t bfType[2]; 
	uint32_t bfSize; 
	uint16_t bfReserved1; 
	uint16_t bfReserved2; 
	uint32_t bfOffBits; 
};

struct BITMAPINFOHEADER
{
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

}

#pragma pack()

Image* ImageFormatBmp::read(Stream* stream)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	Reader reader(stream);

	// Read headers.
	reader >> bmfh.bfType[0];
	reader >> bmfh.bfType[1];
	reader >> bmfh.bfSize;
	reader >> bmfh.bfReserved1;
	reader >> bmfh.bfReserved2;
	reader >> bmfh.bfOffBits;
	
	if (bmfh.bfType[0] != 'B' || bmfh.bfType[1] != 'M')
	{
		log::error << L"Invalid BMP file header" << Endl;
		return 0;
	}
	
	reader >> bmih.biSize;
	reader >> bmih.biWidth;
	reader >> bmih.biHeight;
	reader >> bmih.biPlanes;
	reader >> bmih.biBitCount;
	reader >> bmih.biCompression;
	reader >> bmih.biSizeImage;
	reader >> bmih.biXPelsPerMeter;
	reader >> bmih.biYPelsPerMeter;
	reader >> bmih.biClrUsed;
	reader >> bmih.biClrImportant;

	if (bmih.biCompression != 0)
	{
		log::error << L"Invalid BMP, only BI_RGB supported" << Endl;
		return 0;
	}

	// Get pixel format.
	const PixelFormat* pf;
	switch (bmih.biBitCount)
	{
	case 16:
		pf = PixelFormat::getR5G5B5();
		break;
		
	case 24:
		pf = PixelFormat::getR8G8B8();
		break;
		
	case 32:
		pf = PixelFormat::getX8R8G8B8();
		break;
		
	default:
		log::error << L"Unsupported BMP, must be at least 16 bit color depth (not " << bmih.biBitCount << L")" << Endl;
		return 0;
	}

	Ref< Image > image = gc_new< Image >(pf, bmih.biWidth, bmih.biHeight);

	stream->seek(Stream::SeekSet, bmfh.bfOffBits);

	int bs = pf->getByteSize();
	int rowSize = int(bmih.biWidth * bs);

	char* ptr = static_cast<char*>(image->getData());
	for (int y = 0; y < bmih.biHeight; ++y)
	{
		reader.read(
			&ptr[(bmih.biHeight - y - 1) * bmih.biWidth * bs],
			rowSize
		);
		if (rowSize & 3)
			reader.skip(4 - (rowSize & 3));
	}

	Ref< ImageInfo > imageInfo = gc_new< ImageInfo >();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"BMP");
	image->setImageInfo(imageInfo);

	return image;
}

bool ImageFormatBmp::write(Stream* stream, Image* image)
{
	return false;
}

	}
}
