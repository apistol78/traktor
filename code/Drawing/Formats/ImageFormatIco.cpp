#include "Core/Io/Writer.h"
#include "Core/Math/Log2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Formats/ImageFormatIco.h"
#include "Drawing/Formats/ImageFormatPng.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

int32_t convertDim(int32_t dim)
{
	return dim >= 256 ? 0 : dim;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatIco", ImageFormatIco, IImageFormat)

Ref< Image > ImageFormatIco::read(IStream* stream)
{
	return 0;
}

bool ImageFormatIco::write(IStream* stream, Image* image)
{
	Writer writer(stream);

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();
	if (width > 256 || height > 256)
		return false;

	int32_t lods = log2(max(width, height)) + 1;
	if (lods <= 0)
		return false;

	// ICONDIR header.
	writer << uint8_t(0);	// Reserved
	writer << uint8_t(0);
	writer << uint16_t(1);	// Icon type.
	writer << uint16_t(lods);	// Number of images.

	// ICONDIRENTRY.
	for (int32_t i = 0; i < lods; ++i)
	{
		int32_t lodWidth = max(width >> i, 1);
		int32_t lodHeight = max(height >> i, 1);

		writer << uint8_t(convertDim(lodWidth));
		writer << uint8_t(convertDim(lodHeight));
		writer << uint8_t(0);	// No palette.
		writer << uint8_t(0);	// Reserved
		writer << uint16_t(0);
		writer << uint16_t(0);
		writer << uint32_t(0);	// Size
		writer << uint32_t(0);	// Offset
	}

	// Append PNG data.
	std::vector< int32_t > offsets;
	for (int32_t i = 0; i < lods; ++i)
	{
		int32_t lodWidth = max(width >> i, 1);
		int32_t lodHeight = max(height >> i, 1);

		Ref< drawing::Image > clone = image->clone();
		if (!clone)
			return false;

		ScaleFilter scaleFilter(lodWidth, lodHeight, ScaleFilter::MnAverage, ScaleFilter::MgLinear);
		clone->apply(&scaleFilter);

		offsets.push_back(stream->tell());

		if (!ImageFormatPng().write(stream, clone))
			return false;
	}
	offsets.push_back(stream->tell());

	// Patch offset and size entries.
	int32_t end = stream->tell();
	int32_t entry = 6 + 8;
	for (int32_t i = 0; i < lods; ++i)
	{
		stream->seek(IStream::SeekSet, entry);
		writer << uint32_t(offsets[i + 1] - offsets[i]);
		writer << uint32_t(offsets[i]);
		entry += 16;
	}

	return true;
}

	}
}
