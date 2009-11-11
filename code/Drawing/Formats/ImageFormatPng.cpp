#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_PNG)

#include <png.h>
#include "Drawing/Formats/ImageFormatPng.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{
	namespace drawing
	{

namespace
{

void t_user_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(png_ptr));
	if (stream != 0) stream->write(data, int(length));
}

void t_user_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(png_ptr));
	if (stream != 0) stream->read(data, int(length));
}

void t_user_flush(png_structp png_ptr)
{
	Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(png_ptr));
	if (stream != 0) stream->flush();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatPng", ImageFormatPng, ImageFormat)

Ref< Image > ImageFormatPng::read(Stream* stream)
{
	Ref< Image > image;
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == 0)
		return 0;

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == 0)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	png_set_read_fn(png_ptr, (void *)stream, t_user_read);

	png_read_png(
		png_ptr,
		info_ptr,
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR,
		NULL
	);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	if (bit_depth == 8 && (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA))
	{
		Ref< const PixelFormat > pixelFormat;
#if defined(T_LITTLE_ENDIAN)
		if (color_type == PNG_COLOR_TYPE_RGB)
			pixelFormat = PixelFormat::getR8G8B8();
		else
			pixelFormat = PixelFormat::getA8R8G8B8();
#else
		if (color_type == PNG_COLOR_TYPE_RGB)
			pixelFormat = PixelFormat::getR8G8B8();
		else
			pixelFormat = PixelFormat::getR8G8B8A8();
#endif

		image = gc_new< Image >(pixelFormat, width, height);

		char* data = (char *)image->getData();
		const void** rows = (const void **)png_get_rows(png_ptr, info_ptr);
		for (uint32_t i = 0; i < height; ++i)
		{
			int rowsize = image->getPixelFormat()->getByteSize() * width;
			memcpy(
				data,
				rows[i],
				rowsize
			);
			data += rowsize;
		}

		Ref< ImageInfo > imageInfo = gc_new< ImageInfo >();
		imageInfo->setAuthor(L"Unknown");
		imageInfo->setCopyright(L"Unknown");
		imageInfo->setFormat(L"PNG");
		image->setImageInfo(imageInfo);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return image;
}

bool ImageFormatPng::write(Stream* stream, Image* image)
{
	png_structp png_ptr;
	png_infop info_ptr;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == 0)
		return false;

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == 0)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	png_set_write_fn(png_ptr, (void *)stream, t_user_write, t_user_flush);

	png_set_IHDR(
		png_ptr,
		info_ptr,
		image->getWidth(),
		image->getHeight(),
		8,
		(image->getPixelFormat()->getAlphaBits() > 0) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	Ref< Image > clone = image->clone();
	clone->convert(
		(image->getPixelFormat()->getAlphaBits() > 0) ? PixelFormat::getA8R8G8B8() : PixelFormat::getR8G8B8()
	);

	const char* data = (const char *)clone->getData();
	void** rows = (void **)png_malloc(png_ptr, sizeof(void*) * clone->getHeight());
	for (int32_t i = 0; i < image->getHeight(); ++i)
	{
		int rowsize = clone->getPixelFormat()->getByteSize() * clone->getWidth();
		rows[i] = png_malloc(png_ptr, rowsize);
		memcpy(
			rows[i],
			data,
			rowsize
		);
		data += rowsize;
	}

	png_set_rows(png_ptr, info_ptr, (png_bytepp)rows);
	info_ptr->valid |= PNG_INFO_IDAT;

	png_write_png(
		png_ptr,
		info_ptr,
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR,
		NULL
	);

	for (int32_t i = 0; i < clone->getHeight(); ++i)
		png_free(png_ptr, rows[i]);
	png_free(png_ptr, rows);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	return true;
}

	}
}

#endif	// DRAWING_INCLUDE_PNG
