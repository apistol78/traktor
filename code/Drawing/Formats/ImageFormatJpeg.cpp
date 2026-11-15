/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_JPEG)

#include <setjmp.h>
#include <stdio.h>

extern "C"
{
	#include <jpeglib.h>
}

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Drawing/Formats/ImageFormatJpeg.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{

const int c_bufferSize = 4096;

struct my_source_mgr : jpeg_source_mgr
{
	IStream* stream;
	JOCTET buffer[c_bufferSize];
};

struct my_error_mgr : jpeg_error_mgr
{
	jmp_buf setjmpBuffer;
};

void errorExit(j_common_ptr cinfo)
{
	char message[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, message);
	log::error << L"JPEG decompress error; " << mbstows(message) << Endl;
	longjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer, 1);
}

void outputMessage(j_common_ptr cinfo)
{
	char message[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, message);
	log::warning << L"JPEG decompress warning; " << mbstows(message) << Endl;
}

void initializeSource(j_decompress_ptr cinfo)
{
	my_source_mgr& src = *static_cast< my_source_mgr* >(cinfo->src);
	if (src.bytes_in_buffer)
	{
		src.stream->seek(IStream::SeekCurrent, -int(src.bytes_in_buffer));
		src.next_input_byte = NULL;
		src.bytes_in_buffer = 0;
	}
}

boolean fillInputBuffer(j_decompress_ptr cinfo)
{
	my_source_mgr& src = *static_cast< my_source_mgr* >(cinfo->src);

	int64_t nread = src.stream->read(src.buffer, c_bufferSize);
	if (nread <= 0)
	{
		src.buffer[0] = (JOCTET)0xFF;
		src.buffer[1] = (JOCTET)JPEG_EOI;
		nread = 2;
	}

	src.next_input_byte = src.buffer;
	src.bytes_in_buffer = nread;

	return TRUE;
}

void skipInputData(j_decompress_ptr cinfo, long nbytes)
{
	if (nbytes <= 0)
		return;

	my_source_mgr& src = *static_cast< my_source_mgr* >(cinfo->src);
	if (size_t(nbytes) < src.bytes_in_buffer)
	{
		src.next_input_byte += nbytes;
		src.bytes_in_buffer -= nbytes;
	}
	else
	{
		int skip = int(nbytes - src.bytes_in_buffer);
		src.stream->seek(IStream::SeekCurrent, skip);
		fillInputBuffer(cinfo);
	}
}

void terminateSource(j_decompress_ptr cinfo)
{
	my_source_mgr& src = *static_cast< my_source_mgr* >(cinfo->src);
	if (src.bytes_in_buffer)
	{
		if (src.stream)
			src.stream->seek(IStream::SeekCurrent, -int(src.bytes_in_buffer));
		src.bytes_in_buffer = 0;
		src.next_input_byte = NULL;
	}
	src.stream = 0;
}

bool guardedCreateDecompress(j_decompress_ptr cinfo)
{
	if (setjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer))
		return false;
	jpeg_create_decompress(cinfo);
	return true;
}

bool guardedReadHeader(j_decompress_ptr cinfo, boolean requireImage)
{
	if (setjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer))
		return false;
	jpeg_read_header(cinfo, requireImage);
	return true;
}

bool guardedStartDecompress(j_decompress_ptr cinfo)
{
	if (setjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer))
		return false;
	jpeg_start_decompress(cinfo);
	return true;
}

bool guardedReadScanlines(j_decompress_ptr cinfo, uint8_t* data, uint32_t pitch)
{
	if (setjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer))
		return false;
	while (cinfo->output_scanline < cinfo->output_height)
	{
		JSAMPROW row = (JSAMPROW)(data + cinfo->output_scanline * pitch);
		if (jpeg_read_scanlines(cinfo, &row, 1) != 1)
			return false;
	}
	return true;
}

bool guardedFinishDecompress(j_decompress_ptr cinfo)
{
	if (setjmp(static_cast< my_error_mgr* >(cinfo->err)->setjmpBuffer))
		return false;
	jpeg_finish_decompress(cinfo);
	return true;
}

class ImageFormatJpegImpl
{
public:
	ImageFormatJpegImpl();

	~ImageFormatJpegImpl();

	bool readJpegHeader(IStream* stream);

	Ref< Image > readJpegImage(IStream* stream);

private:
	struct jpeg_decompress_struct m_cinfo;
	struct my_error_mgr m_jerr;
	struct my_source_mgr m_pub;
	bool m_created = false;

	void reset();
};

ImageFormatJpegImpl::ImageFormatJpegImpl()
{
	m_cinfo.err = jpeg_std_error(&m_jerr);
	m_cinfo.err->error_exit = errorExit;
	m_cinfo.err->output_message = outputMessage;

	m_created = guardedCreateDecompress(&m_cinfo);
	if (!m_created)
		return;

	m_pub.init_source = initializeSource;
	m_pub.fill_input_buffer = fillInputBuffer;
	m_pub.skip_input_data = skipInputData;
	m_pub.resync_to_restart = jpeg_resync_to_restart;
	m_pub.term_source = terminateSource;
	m_pub.bytes_in_buffer = 0;
	m_pub.next_input_byte = NULL;
	m_cinfo.src = static_cast< jpeg_source_mgr* >(&m_pub);
}

ImageFormatJpegImpl::~ImageFormatJpegImpl()
{
	if (m_created)
		jpeg_destroy_decompress(&m_cinfo);
}

void ImageFormatJpegImpl::reset()
{
	m_pub.bytes_in_buffer = 0;
	m_pub.next_input_byte = NULL;
	jpeg_abort_decompress(&m_cinfo);
}

bool ImageFormatJpegImpl::readJpegHeader(IStream* stream)
{
	if (!m_created)
		return false;

	T_ASSERT(m_pub.bytes_in_buffer == 0);
	m_pub.stream = stream;

	if (!guardedReadHeader(&m_cinfo, FALSE))
	{
		reset();
		return false;
	}

	// Roll back unused bytes.
	if (m_pub.bytes_in_buffer)
	{
		m_pub.stream->seek(IStream::SeekCurrent, -int(m_pub.bytes_in_buffer));
		m_pub.bytes_in_buffer = 0;
		m_pub.next_input_byte = NULL;
	}

	return true;
}

Ref< Image > ImageFormatJpegImpl::readJpegImage(IStream* stream)
{
	if (!m_created)
		return nullptr;

	T_ASSERT(m_pub.bytes_in_buffer == 0);
	m_pub.stream = stream;

	if (m_cinfo.global_state == /*DSTATE_START*/200 || m_cinfo.global_state == /*DSTATE_INHEADER*/201)
	{
		if (!guardedReadHeader(&m_cinfo, TRUE))
		{
			reset();
			return nullptr;
		}
	}

	if (m_cinfo.global_state != /*DSTATE_READY*/202)
		return nullptr;

	if (!guardedStartDecompress(&m_cinfo))
	{
		reset();
		return nullptr;
	}

	PixelFormat pixelFormat;
	if (m_cinfo.output_components == 4)
		pixelFormat = PixelFormat::getX8B8G8R8();
	else if (m_cinfo.output_components == 3)
#if defined(T_LITTLE_ENDIAN)
		pixelFormat = PixelFormat::getB8G8R8();
#elif defined(T_BIG_ENDIAN)
		pixelFormat = PixelFormat::getR8G8B8();
#endif
	else if (m_cinfo.output_components == 1)
		pixelFormat = PixelFormat::getR8();
	else
	{
		log::error << L"Unsupported number of components, must be either 3 or 4" << Endl;
		reset();
		return nullptr;
	}

	Ref< Image > image = new Image(
		pixelFormat,
		m_cinfo.output_width,
		m_cinfo.output_height
	);

	if (!guardedReadScanlines(&m_cinfo, (uint8_t*)image->getData(), m_cinfo.output_width * m_cinfo.output_components))
	{
		reset();
		return nullptr;
	}

	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"JPEG");
	imageInfo->setGamma(2.2f);
	image->setImageInfo(imageInfo);

	if (!guardedFinishDecompress(&m_cinfo))
	{
		reset();
		return nullptr;
	}

	// Roll back unused bytes.
	if (m_pub.bytes_in_buffer)
	{
		m_pub.stream->seek(IStream::SeekCurrent, -int(m_pub.bytes_in_buffer));
		m_pub.bytes_in_buffer = 0;
		m_pub.next_input_byte = NULL;
	}

	return image;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatJpeg", ImageFormatJpeg, IImageFormat)

ImageFormatJpeg::ImageFormatJpeg()
:	m_impl(new ImageFormatJpegImpl())
{
}

ImageFormatJpeg::~ImageFormatJpeg()
{
	delete m_impl;
}

Ref< Image > ImageFormatJpeg::read(IStream* stream)
{
	return m_impl->readJpegImage(stream);
}

bool ImageFormatJpeg::write(IStream* stream, const Image* image)
{
	return false;
}

bool ImageFormatJpeg::readJpegHeader(IStream* stream)
{
	return m_impl->readJpegHeader(stream);
}

Ref< Image > ImageFormatJpeg::readJpegImage(IStream* stream)
{
	return m_impl->readJpegImage(stream);
}

}

#endif	// DRAWING_INCLUDE_JPEG
