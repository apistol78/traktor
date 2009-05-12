#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Palette.h"
#include "Drawing/ImageFormat.h"
#include "Drawing/ImageFilter.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

const uint32_t c_wallSize = 64;

void checkData(uint8_t* ptr, size_t size)
{
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
}

uint8_t* allocData(size_t size)
{
	uint8_t* ptr = new uint8_t [size + c_wallSize * 2];
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
		delete[] ptr;
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Image", Image, Object)

Image::Image()
:	m_width(0)
,	m_height(0)
,	m_data(0)
{
}

Image::Image(const Image& src)
:	m_pixelFormat(src.m_pixelFormat)
,	m_width(src.m_width)
,	m_height(src.m_height)
,	m_palette(src.m_palette)
,	m_size(0)
,	m_data(0)
,	m_imageInfo(src.m_imageInfo)
{
	if (m_pixelFormat)
	{
		m_size = src.m_size;
		m_data = allocData(m_size);
		std::memcpy(m_data, src.m_data, m_size);
	}
}

Image::Image(const PixelFormat* pixelFormat, uint32_t width, uint32_t height, Palette* palette)
:	m_pixelFormat(pixelFormat)
,	m_width(width)
,	m_height(height)
,	m_palette(palette)
,	m_size(0)
,	m_data(0)
{
	if (m_pixelFormat)
	{
		m_size = m_width * m_height * m_pixelFormat->getByteSize();
		m_data = allocData(m_size);
		std::memset(m_data, 0, m_size);
	}
}

Image::~Image()
{
	freeData(m_data, m_size);
}

Image* Image::clone(bool includeData) const
{
	Ref< Image > clone = gc_new< Image >(m_pixelFormat, m_width, m_height, m_palette);
	if (includeData)
	{
		std::memcpy(clone->m_data, m_data, m_width * m_height * m_pixelFormat->getByteSize());
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

	const PixelFormat* pf = src->m_pixelFormat;

	uint32_t srcPitch = pf->getByteSize() * src->m_width;
	uint32_t dstPitch = m_pixelFormat->getByteSize() * m_width;

	for (int32_t yy = 0; yy < height; ++yy)
	{
		const uint8_t* sd = &src->m_data[x * pf->getByteSize() + (y + yy) * srcPitch];
		uint8_t* dd = &m_data[yy * dstPitch];
		pf->convert(0, sd, m_pixelFormat, 0, dd, width);
	}

	checkData(m_data, m_size);
}

void Image::clear(const Color& color)
{
	float tmp[] =
	{
		color.getRed(),
		color.getGreen(),
		color.getBlue(),
		color.getAlpha()
	};
	
	uint32_t byteSize = m_pixelFormat->getByteSize();
	std::vector< uint8_t > c(byteSize);

	PixelFormat::getRGBAF32()->convert(
		0,
		tmp,
		m_pixelFormat,
		m_palette,
		&c[0],
		1
	);

	for (uint32_t i = 0; i < m_width * m_height * byteSize; i += byteSize)
		std::memcpy(&m_data[i], &c[0], byteSize);

	checkData(m_data, m_size);
}

bool Image::getPixel(uint32_t x, uint32_t y, Color& color) const
{
	if (x >= m_width || y >= m_height)
		return false;

	float tmp[] =
	{
		0,
		0,
		0,
		0
	};
	
	m_pixelFormat->convert(
		m_palette,
		&m_data[(x + y * m_width) * m_pixelFormat->getByteSize()],
		PixelFormat::getRGBAF32(),
		0,
		tmp,
		1
	);
	
	color.set(tmp[0], tmp[1], tmp[2], tmp[3]);
	return true;
}

bool Image::setPixel(uint32_t x, uint32_t y, const Color& color)
{
	if (x >= m_width || y >= m_height)
		return false;

	float tmp[] =
	{
		color.getRed(),
		color.getGreen(),
		color.getBlue(),
		color.getAlpha()
	};
	
	PixelFormat::getRGBAF32()->convert(
		0,
		tmp,
		m_pixelFormat,
		m_palette,
		&m_data[(x + y * m_width) * m_pixelFormat->getByteSize()],
		1
	);
	
	checkData(m_data, m_size);
	return true;
}

Image* Image::applyFilter(ImageFilter* imageFilter) const
{
	Ref< Image > image = imageFilter->apply(this);
	checkData(m_data, m_size);
	checkData(image->m_data, image->m_size);
	return image;
}

void Image::convert(const PixelFormat* intoPixelFormat, Palette* intoPalette)
{
	size_t size = m_width * m_height * intoPixelFormat->getByteSize();
	uint8_t* tmp = allocData(size);

	m_pixelFormat->convert(
		m_palette,
		m_data,
		intoPixelFormat,
		intoPalette,
		tmp,
		m_width * m_height
	);

	freeData(m_data, m_size);

	m_size = size;
	m_data = tmp;
	m_pixelFormat = intoPixelFormat;

	checkData(m_data, m_size);
}

Image* Image::load(const Path& fileName)
{
	Ref< Image > image;

	Ref< ImageFormat > imageFormat = ImageFormat::determineFormat(fileName);
	if (imageFormat == 0)
		return 0;

	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (file == 0)
		return 0;

	image = imageFormat->read(file);

	if (image)
		checkData(image->m_data, image->m_size);

	file->close();
	return image;
}

Image* Image::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< Image > image;

	Ref< ImageFormat > imageFormat = ImageFormat::determineFormat(extension);
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

	Ref< ImageFormat > imageFormat = ImageFormat::determineFormat(fileName);
	if (imageFormat == 0)
		return false;

	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (file == 0)
		return false;

	bool result = imageFormat->write(file, this);
	checkData(m_data, m_size);

	file->close();
	return result;
}

const PixelFormat* Image::getPixelFormat() const
{
	return m_pixelFormat;
}

const uint32_t Image::getWidth() const
{
	return m_width;
}

const uint32_t Image::getHeight() const
{
	return m_height;
}

Palette* Image::getPalette() const
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

ImageInfo* Image::getImageInfo() const
{
	return m_imageInfo;
}

Image& Image::operator = (const Image& src)
{
	freeData(m_data, m_size);

	m_pixelFormat = src.m_pixelFormat;
	m_width = src.m_width;
	m_height = src.m_height;
	m_palette = src.m_palette;
	m_size = 0;
	m_data = 0;
	m_imageInfo = src.m_imageInfo;

	if (m_pixelFormat)
	{
		m_size = src.m_size;
		m_data = allocData(m_size);
		memcpy(m_data, src.m_data, m_size);
	}

	checkData(m_data, m_size);
	return *this;
}

	}
}
