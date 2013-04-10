#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
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

Ref< Heightfield > HeightfieldFormat::read(IStream* stream, const Vector4& worldExtent) const
{
	Ref< drawing::Image > heightfieldImage;

	if ((heightfieldImage = readRawTerrain(stream)) == 0)
		return 0;

	uint32_t size = heightfieldImage->getWidth();

	Ref< Heightfield > heightfield = new Heightfield(
		size,
		worldExtent
	);

	const height_t* sourceHeights = static_cast< const height_t* >(heightfieldImage->getData());
	height_t* destinationHeights = heightfield->getHeights();

	std::memcpy(
		destinationHeights,
		sourceHeights,
		size * size * sizeof(height_t)
	);

	return heightfield;
}

bool HeightfieldFormat::write(IStream* stream, const Heightfield* heightfield) const
{
	Writer(stream).write(
		heightfield->getHeights(),
		heightfield->getSize() * heightfield->getSize(),
		sizeof(height_t)
	);
	return true;
}

	}
}
