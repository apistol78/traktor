#include <cstring>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Drawing/Image.h"
#include "Drawing/Palette.h"
#include "Drawing/IImageFormat.h"
#include "Drawing/IImageFilter.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

#if defined(DRAWING_CHECK_DATA)
const uint32_t c_wallSize = 64;
#else
const uint32_t c_wallSize = 0;
#endif

void checkData(uint8_t* ptr, size_t size)
{
#if defined(DRAWING_CHECK_DATA)
	if (!ptr || !size)
		return;

	uint8_t* wp = ptr - c_wallSize;
	for (uint32_t i = 0; i < c_wallSize; ++i)
	{
		if (wp[i] != 0xcc)
			T_FATAL_ERROR;
		if (wp[c_wallSize + size + i] != 0x55)
			T_FATAL_ERROR;
	}
#endif
}

uint8_t* allocData(size_t size)
{
	uint8_t* ptr = (uint8_t*)getAllocator()->alloc(size + c_wallSize * 2, 16, T_FILE_LINE);
	if (!ptr)
		T_FATAL_ERROR;
	for (uint32_t i = 0; i < c_wallSize; ++i)
	{
		ptr[i] = 0xcc;
		ptr[c_wallSize + size + i] = 0x55;
	}
	return ptr + c_wallSize;
}

void freeData(uint8_t* ptr, size_t size)
{
	if (ptr)
	{
		checkData(ptr, size);
		ptr -= c_wallSize;
		getAllocator()->free(ptr);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Image", Image, Object)

Image::Image()
:	m_width(0)
,	m_height(0)
,	m_pitch(0)
,	m_data(0)
{
}

Image::Image(const Image& src)
:	m_pixelFormat(src.m_pixelFormat)
,	m_width(src.m_width)
,	m_height(src.m_height)
,	m_pitch(src.m_pitch)
,	m_palette(src.m_palette)
,	m_size(0)
,	m_data(0)
,	m_imageInfo(src.m_imageInfo)
{
	m_size = src.m_size;
	m_data = allocData(m_size);
	std::memcpy(m_data, src.m_data, m_size);
}

Image::Image(const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette)
:	m_pixelFormat(pixelFormat)
,	m_width(width)
,	m_height(height)
,	m_pitch(width * pixelFormat.getByteSize())
,	m_palette(palette)
,	m_size(0)
,	m_data(0)
{
	m_size = m_height * m_pitch;
	m_data = allocData(m_size);
	std::memset(m_data, 0, m_size);
}

Image::~Image()
{
	freeData(m_data, m_size);
}

Ref< Image > Image::clone(bool includeData) const
{
	Ref< Image > clone = new Image(m_pixelFormat, m_width, m_height, m_palette);
	if (includeData)
	{
		std::memcpy(clone->m_data, m_data, m_height * m_pitch);
		checkData(clone->m_data, clone->m_size);
	}
	return clone;
}

void Image::copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height)
{
	int32_t srcWidth = int32_t(src->getWidth());
	int32_t srcHeight = int32_t(src->getHeight());

	if (x >= srcWidth || y >= srcHeight)
		return;

	if (x < 0)
	{
		width -= -x;
		x = 0;
	}
	if (y < 0)
	{
		height -= y;
		y = 0;
	}

	if (x + width > srcWidth)
		width = srcWidth - x;
	if (y + height > srcHeight)
		height = srcHeight - y;

	if (width > int32_t(getWidth()))
		width = int32_t(getWidth());
	if (height > int32_t(getHeight()))
		height = int32_t(getHeight());

	if (width <= 0 || height <= 0)
		return;

	T_ASSERT (x >= 0 && y >= 0);
	T_ASSERT (width >= 0 && height >= 0);

	const PixelFormat& pf = src->m_pixelFormat;
	for (int32_t yy = 0; yy < height; ++yy)
	{
		const uint8_t* sd = &src->m_data[x * pf.getByteSize() + (y + yy) * src->m_pitch];
		uint8_t* dd = &m_data[yy * m_pitch];
		pf.convert(0, sd, m_pixelFormat, 0, dd, width);
	}

	checkData(m_data, m_size);
}

void Image::clear(const Color4f& color)
{
	float tmp[] =
	{
		color.getRed(),
		color.getGreen(),
		color.getBlue(),
		color.getAlpha()
	};
	
	uint32_t byteSize = m_pixelFormat.getByteSize();
	std::vector< uint8_t > c(byteSize);

	PixelFormat::getRGBAF32().convert(
		0,
		tmp,
		m_pixelFormat,
		m_palette,
		&c[0],
		1
	);

	for (int32_t i = 0; i < m_height * m_pitch; i += byteSize)
		std::memcpy(&m_data[i], &c[0], byteSize);

	checkData(m_data, m_size);
}

bool Image::getPixel(int32_t x, int32_t y, Color4f& outColor) const
{
	if (x < 0  || x >= m_width || y < 0 || y >= m_height)
		return false;

	getPixelUnsafe(x, y, outColor);
	return true;
}

bool Image::setPixel(int32_t x, int32_t y, const Color4f& color)
{
	if (x < 0  || x >= m_width || y < 0 || y >= m_height)
		return false;

	setPixelUnsafe(x, y, color);
	return true;
}

void Image::getPixelUnsafe(int32_t x, int32_t y, Color4f& outColor) const
{
	float T_MATH_ALIGN16 tmp[4];
	
	m_pixelFormat.convert(
		m_palette,
		&m_data[x * m_pixelFormat.getByteSize() + y * m_pitch],
		PixelFormat::getRGBAF32(),
		0,
		tmp,
		1
	);
	
	outColor = Color4f::loadAligned(tmp);
}

void Image::setPixelUnsafe(int32_t x, int32_t y, const Color4f& color)
{
	float T_MATH_ALIGN16 tmp[4];
	color.storeAligned(tmp);
	PixelFormat::getRGBAF32().convert(
		0,
		tmp,
		m_pixelFormat,
		m_palette,
		&m_data[x * m_pixelFormat.getByteSize() + y * m_pitch],
		1
	);
}

void Image::getSpanUnsafe(int32_t y, Color4f* outSpan) const
{
	float T_MATH_ALIGN16 tmp[64 * 4];
	int32_t offset = y * m_pitch;
	int32_t x = 0;
	for (; x < m_width - 64; x += 64)
	{
		m_pixelFormat.convert(
			m_palette,
			&m_data[x * m_pixelFormat.getByteSize() + offset],
			PixelFormat::getRGBAF32(),
			0,
			tmp,
			64
		);
		for (int32_t i = 0; i < 64; i += 4)
		{
			int32_t ii = i << 2;
			Color4f c0 = Color4f::loadAligned(&tmp[ii + 0]);
			Color4f c1 = Color4f::loadAligned(&tmp[ii + 4]);
			Color4f c2 = Color4f::loadAligned(&tmp[ii + 8]);
			Color4f c3 = Color4f::loadAligned(&tmp[ii + 12]);
			outSpan[x + i + 0] = c0;
			outSpan[x + i + 1] = c1;
			outSpan[x + i + 2] = c2;
			outSpan[x + i + 3] = c3;
		}
	}
	if (x < m_width)
	{
		int32_t n = m_width - x;
		T_ASSERT (n < 64);

		m_pixelFormat.convert(
			m_palette,
			&m_data[x * m_pixelFormat.getByteSize() + offset],
			PixelFormat::getRGBAF32(),
			0,
			tmp,
			n
		);
		for (int32_t i = 0; i < n; ++i)
			outSpan[x + i] = Color4f::loadAligned(&tmp[i << 2]);
	}
}

void Image::setSpanUnsafe(int32_t y, const Color4f* span)
{
	float T_MATH_ALIGN16 tmp[64 * 4];
	int32_t offset = y * m_pitch;
	int32_t x = 0;
	for (; x < m_width - 64; x += 64)
	{
		for (int32_t i = 0; i < 64; i += 4)
		{
			int32_t ii = i << 2;
			span[x + i + 0].storeAligned(&tmp[ii + 0]);
			span[x + i + 1].storeAligned(&tmp[ii + 4]);
			span[x + i + 2].storeAligned(&tmp[ii + 8]);
			span[x + i + 3].storeAligned(&tmp[ii + 12]);
		}
		PixelFormat::getRGBAF32().convert(
			0,
			tmp,
			m_pixelFormat,
			m_palette,
			&m_data[x * m_pixelFormat.getByteSize() + offset],
			64
		);
	}
	if (x < m_width)
	{
		int32_t n = m_width - x;
		T_ASSERT (n < 64);

		for (int32_t i = 0; i < n; ++i)
			span[x + i].storeAligned(&tmp[i << 2]);
		PixelFormat::getRGBAF32().convert(
			0,
			tmp,
			m_pixelFormat,
			m_palette,
			&m_data[x * m_pixelFormat.getByteSize() + offset],
			n
		);
	}
}

Ref< Image > Image::applyFilter(IImageFilter* imageFilter) const
{
	Ref< Image > image = imageFilter->apply(this);
	checkData(m_data, m_size);
	checkData(image->m_data, image->m_size);
	return image;
}

void Image::convert(const PixelFormat& intoPixelFormat, Palette* intoPalette)
{
	size_t size = m_width * m_height * intoPixelFormat.getByteSize();
	uint8_t* tmp = allocData(size);

	m_pixelFormat.convert(
		m_palette,
		m_data,
		intoPixelFormat,
		intoPalette,
		tmp,
		m_width * m_height
	);

	freeData(m_data, m_size);

	m_pitch = m_width * intoPixelFormat.getByteSize();
	m_size = size;
	m_data = tmp;
	m_pixelFormat = intoPixelFormat;

	checkData(m_data, m_size);
}

Ref< Image > Image::load(const Path& fileName)
{
	Ref< Image > image;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(fileName);
	if (imageFormat == 0)
		return 0;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (file == 0)
		return 0;

	BufferedStream bufferedFile(file);

	image = imageFormat->read(&bufferedFile);

	if (image)
		checkData(image->m_data, image->m_size);

	file->close();
	return image;
}

Ref< Image > Image::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< Image > image;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(extension);
	if (imageFormat == 0)
		return 0;

	MemoryStream stream(resource, size);
	image = imageFormat->read(&stream);

	if (image)
		checkData(image->m_data, image->m_size);

	return image;
}

bool Image::save(const Path& fileName)
{
	if (!m_data)
		return false;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(fileName);
	if (imageFormat == 0)
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (file == 0)
		return false;

	bool result = imageFormat->write(file, this);
	checkData(m_data, m_size);

	file->close();
	return result;
}

const PixelFormat& Image::getPixelFormat() const
{
	return m_pixelFormat;
}

const int32_t Image::getWidth() const
{
	return m_width;
}

const int32_t Image::getHeight() const
{
	return m_height;
}

Ref< Palette > Image::getPalette() const
{
	return m_palette;
}

const void* Image::getData() const
{
	return m_data;
}

void* Image::getData()
{
	return m_data;
}

uint32_t Image::getDataSize() const
{
	return m_size;
}

void Image::setImageInfo(ImageInfo* imageInfo)
{
	m_imageInfo = imageInfo;
}

Ref< ImageInfo > Image::getImageInfo() const
{
	return m_imageInfo;
}

Image& Image::operator = (const Image& src)
{
	freeData(m_data, m_size);

	m_pixelFormat = src.m_pixelFormat;
	m_width = src.m_width;
	m_height = src.m_height;
	m_pitch = src.m_pitch;
	m_palette = src.m_palette;
	m_imageInfo = src.m_imageInfo;

	m_size = src.m_size;
	m_data = allocData(m_size);
	std::memcpy(m_data, src.m_data, m_size);

	checkData(m_data, m_size);
	return *this;
}

	}
}
