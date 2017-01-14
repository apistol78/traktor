#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_JPEG)

#include <stdio.h>

extern "C"
{
	#include <jpeglib.h>
}

#include "Drawing/Formats/ImageFormatJpeg.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace drawing
	{

const int c_bufferSize = 4096;
static bool s_errorOccured = false;

struct my_source_mgr : jpeg_source_mgr
{
	IStream* stream;
	JOCTET buffer[c_bufferSize];
};

void errorExit(j_common_ptr cinfo)
{
	log::error << L"JPEG decompress error" << Endl;
	s_errorOccured = true;
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
	
	int nread = src.stream->read(src.buffer, c_bufferSize);
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

class ImageFormatJpegImpl
{
public:
	ImageFormatJpegImpl();

	~ImageFormatJpegImpl();

	bool readJpegHeader(IStream* stream);

	Ref< Image > readJpegImage(IStream* stream);

private:
	struct jpeg_decompress_struct m_cinfo;
	struct jpeg_error_mgr m_jerr;
	struct my_source_mgr m_pub;
};

ImageFormatJpegImpl::ImageFormatJpegImpl()
{
	s_errorOccured = false;

	m_cinfo.err = jpeg_std_error(&m_jerr);
	m_cinfo.err->error_exit = errorExit;

	jpeg_create_decompress(&m_cinfo);
	T_ASSERT (!s_errorOccured);

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
	jpeg_destroy_decompress(&m_cinfo);
}

bool ImageFormatJpegImpl::readJpegHeader(IStream* stream)
{
	T_ASSERT (m_pub.bytes_in_buffer == 0);
	m_pub.stream = stream;

	jpeg_read_header(&m_cinfo, FALSE);

	// Roll back unused bytes.
	if (m_pub.bytes_in_buffer)
	{
		m_pub.stream->seek(IStream::SeekCurrent, -int(m_pub.bytes_in_buffer));
		m_pub.bytes_in_buffer = 0;
		m_pub.next_input_byte = NULL;
	}

	return !s_errorOccured;
}

Ref< Image > ImageFormatJpegImpl::readJpegImage(IStream* stream)
{
	T_ASSERT (m_pub.bytes_in_buffer == 0);
	m_pub.stream = stream;

	if (m_cinfo.global_state == /*DSTATE_START*/200 || m_cinfo.global_state == /*DSTATE_INHEADER*/201)
	{
		jpeg_read_header(&m_cinfo, TRUE);
		if (s_errorOccured)
			return 0;
	}

	if (m_cinfo.global_state != /*DSTATE_READY*/202)
		return 0;

	jpeg_start_decompress(&m_cinfo); 
	if (s_errorOccured)
		return 0;

	PixelFormat pixelFormat;
	if (m_cinfo.output_components == 4)
		pixelFormat = PixelFormat::getX8B8G8R8();
	else if (m_cinfo.output_components == 3)
#if defined(T_LITTLE_ENDIAN)
		pixelFormat = PixelFormat::getB8G8R8();
#elif defined(T_BIG_ENDIAN)
		pixelFormat = PixelFormat::getR8G8B8();
#endif
	else
	{
		log::error << L"Unsupported number of components, must be either 3 or 4" << Endl;
		return 0;
	}

	Ref< Image > image = new Image(
		pixelFormat,
		m_cinfo.output_width,
		m_cinfo.output_height
	);

	uint8_t* data = (uint8_t *)image->getData();
	for (uint32_t y = 0; y < m_cinfo.output_height; ++y)
	{
#if defined(_DEBUG)
		int linesRead =
#endif
		jpeg_read_scanlines(&m_cinfo, &data, 1);
		T_ASSERT (linesRead == 1);
		data += m_cinfo.output_width * m_cinfo.output_components;
	}

	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setAuthor(L"Unknown");
	imageInfo->setCopyright(L"Unknown");
	imageInfo->setFormat(L"JPEG");
	image->setImageInfo(imageInfo);

	jpeg_finish_decompress(&m_cinfo);

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

bool ImageFormatJpeg::write(IStream* stream, Image* image)
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
}

#endif	// DRAWING_INCLUDE_JPEG
