#include <cstring>
#include "Core/Io/IStream.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldFormat.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

Ref< drawing::Image > readRawTerrain(IStream* stream)
{
	uint32_t fileSize = stream->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR16(),
		size,
		size
	);

	stream->read(image->getData(), fileSize);
	stream->close();

	return image;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldFormat", HeightfieldFormat, Object)

Ref< Heightfield > HeightfieldFormat::read(IStream* stream, const std::wstring& extension, const Vector4& worldExtent, bool invertX, bool invertZ, uint32_t detailSkip) const
{
	Ref< drawing::Image > heightfieldImage;

	// Load base layer as image.
	if ((heightfieldImage = readRawTerrain(stream)) == 0)
		return 0;

	// Flip base image.
	if (invertX || invertZ)
	{
		drawing::MirrorFilter mirrorFilter(invertX, invertZ);
		heightfieldImage = heightfieldImage->applyFilter(&mirrorFilter);
		T_ASSERT (heightfieldImage);
	}

	uint32_t size = heightfieldImage->getWidth();

	// Rescale base layer to fit 2^x.
	if (detailSkip > 1)
	{
		if (!(size /= detailSkip))
			return 0;

		drawing::ScaleFilter scaleFilter(
			size,
			size,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		heightfieldImage = heightfieldImage->applyFilter(&scaleFilter);
		T_ASSERT (heightfieldImage);
	}

	// Create heightfield.
	Ref< Heightfield > heightfield = new Heightfield(
		size,
		worldExtent
	);

	// Copy heights into heightfield.
	const height_t* sourceHeights = static_cast< const height_t* >(heightfieldImage->getData());
	height_t* destinationHeights = heightfield->getHeights();

	std::memcpy(
		destinationHeights,
		sourceHeights,
		size * size * sizeof(height_t)
	);

	return heightfield;
}

	}
}
