#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

const int32_t c_version = 2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldFormat", HeightfieldFormat, Object)

Ref< Heightfield > HeightfieldFormat::read(IStream* stream, const Vector4& worldExtent) const
{
	int32_t version;
	Reader(stream) >> version;
	
	if (version != 1 && version != 2)
		return 0;

	int32_t size;
	Reader(stream) >> size;

	Ref< Heightfield > heightfield = new Heightfield(
		size,
		worldExtent
	);

	height_t* heights = heightfield->getHeights();
	T_ASSERT_M (heights, L"No heights in heightfield");
	Reader(stream).read(heights, size * size, sizeof(height_t));

	uint8_t* cuts = heightfield->getCuts();
	T_ASSERT_M (cuts, L"No cuts in heightfield");
	Reader(stream).read(cuts, size * size / 8, sizeof(uint8_t));

	uint8_t* material = heightfield->getMaterial();
	T_ASSERT_M (material, L"No material in heightfield");
	if (version == 2)
		Reader(stream).read(material, size * size, sizeof(uint8_t));
	else
		std::memset(material, 0, size * size * sizeof(uint8_t));

	stream->close();
	return heightfield;
}

bool HeightfieldFormat::write(IStream* stream, const Heightfield* heightfield) const
{
	Writer(stream) << int32_t(c_version);
	Writer(stream) << int32_t(heightfield->getSize());

	Writer(stream).write(
		heightfield->getHeights(),
		heightfield->getSize() * heightfield->getSize(),
		sizeof(height_t)
	);

	Writer(stream).write(
		heightfield->getCuts(),
		heightfield->getSize() * heightfield->getSize() / 8,
		sizeof(uint8_t)
	);

	Writer(stream).write(
		heightfield->getMaterial(),
		heightfield->getSize() * heightfield->getSize(),
		sizeof(uint8_t)
	);

	return true;
}

	}
}
