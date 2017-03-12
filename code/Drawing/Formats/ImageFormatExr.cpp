#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_EXR)

#include <ImfIO.h>
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h> 
#include <ImfFrameBuffer.h>
#include <half.h>
#include "Drawing/Formats/ImageFormatExr.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

class IStreamWrapper : public Imf::IStream
{
public:
	IStreamWrapper(traktor::IStream* stream)
	:	Imf::IStream("n/a")
	,	m_stream(stream)
	{
	}

	virtual bool read(char c[], int n)
	{
		return bool(m_stream->read(c, n) == n);
	}

	virtual Imf::Int64 tellg()
	{
		return Imf::Int64(m_stream->tell());
	}

	virtual void seekg(Imf::Int64 pos)
	{
		m_stream->seek(traktor::IStream::SeekSet, int64_t(pos));
	}

private:
	Ref< traktor::IStream > m_stream;
};

class OStreamWrapper : public Imf::OStream
{
public:
	OStreamWrapper(traktor::IStream* stream)
	:	Imf::OStream("n/a")
	,	m_stream(stream)
	{
	}

	virtual void write(const char c[], int n)
	{
		m_stream->write(c, n);
	}

	virtual Imf::Int64 tellp()
	{
		return Imf::Int64(m_stream->tell());
	}

	virtual void seekp(Imf::Int64 pos)
	{
		m_stream->seek(traktor::IStream::SeekSet, int64_t(pos));
	}

private:
	Ref< traktor::IStream > m_stream;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatExr", ImageFormatExr, IImageFormat)

Ref< Image > ImageFormatExr::read(IStream* stream)
{
	IStreamWrapper is(stream);
	Imf::InputFile file(is);

	Imath::Box2i dw = file.header().dataWindow();
	int sizex = dw.max.x - dw.min.x + 1;
	int sizey = dw.max.y - dw.min.y + 1;

	std::vector< half > rgb(4 * sizex * sizey);

	Imf::FrameBuffer frameBuffer;
	frameBuffer.insert("R", Imf::Slice(Imf::HALF, (char *)&rgb[0], 4 * sizeof(half), sizex * 4 * sizeof(half), 1, 1, 0.0f));
	frameBuffer.insert("G", Imf::Slice(Imf::HALF, (char *)&rgb[1], 4 * sizeof(half), sizex * 4 * sizeof(half), 1, 1, 0.0f));
	frameBuffer.insert("B", Imf::Slice(Imf::HALF, (char *)&rgb[2], 4 * sizeof(half), sizex * 4 * sizeof(half), 1, 1, 0.0f));
	frameBuffer.insert("A", Imf::Slice(Imf::HALF, (char *)&rgb[3], 4 * sizeof(half), sizex * 4 * sizeof(half), 1, 1, 0.0f));
	file.setFrameBuffer(frameBuffer);
	file.readPixels(dw.min.y, dw.max.y);

	Ref< drawing::Image > image = new drawing::Image(
		PixelFormat::getRGBAF32(),
		sizex,
		sizey
	);

	const half* s = &rgb[0];
	float* d = static_cast< float* >(image->getData());

	for(int y = 0; y < sizey; ++y)
	{
		for(int x = 0; x < sizex; ++x)
		{
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
		}
	}

	return image;
}

bool ImageFormatExr::write(IStream* stream, Image* image)
{
	OStreamWrapper os(stream);

	Ref< Image > clone = image->clone();
	if (!clone)
		return false;

	clone->convert(PixelFormat::getRGBAF32());

	Imf::Header header(clone->getWidth(), clone->getHeight());
	header.channels().insert("R", Imf::Channel(Imf::HALF));
	header.channels().insert("G", Imf::Channel(Imf::HALF));
	header.channels().insert("B", Imf::Channel(Imf::HALF));
	header.channels().insert("A", Imf::Channel(Imf::HALF));
	header.compression() = Imf::Compression();

	std::vector< half > rgb(4 * clone->getWidth() * clone->getHeight());

	const float* s = static_cast< const float* >(image->getData());
	half* d = &rgb[0];

	for(int y = 0; y < clone->getHeight(); ++y)
	{
		for(int x = 0; x < clone->getWidth(); ++x)
		{
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
		}
	}

	Imf::FrameBuffer frameBuffer;
	frameBuffer.insert("R", Imf::Slice(Imf::HALF, (char *)&rgb[0], 4 * sizeof(half), clone->getWidth() * 4 * sizeof(half)));
	frameBuffer.insert("G", Imf::Slice(Imf::HALF, (char *)&rgb[1], 4 * sizeof(half), clone->getWidth() * 4 * sizeof(half)));
	frameBuffer.insert("B", Imf::Slice(Imf::HALF, (char *)&rgb[2], 4 * sizeof(half), clone->getWidth() * 4 * sizeof(half)));
	frameBuffer.insert("A", Imf::Slice(Imf::HALF, (char *)&rgb[3], 4 * sizeof(half), clone->getWidth() * 4 * sizeof(half)));
	
	Imf::OutputFile file(os, header);
	file.setFrameBuffer(frameBuffer);
	file.writePixels(clone->getHeight());

	return false;
}

	}
}

#endif	// DRAWING_INCLUDE_EXR
