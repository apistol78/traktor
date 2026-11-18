/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Misc/AutoPtr.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"

namespace traktor::hf
{
	namespace
	{

const int32_t c_version = 5;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldFormat", HeightfieldFormat, Object)

Ref< Heightfield > HeightfieldFormat::read(IStream* stream, const Vector4& worldExtent, const Vector4& worldCenter) const
{
	int32_t version;
	Reader(stream) >> version;

	if (version < 1 || version > c_version)
		return 0;

	int32_t size;
	Reader(stream) >> size;

	uint32_t layers = 0;
	if (version >= 5)
		Reader(stream) >> layers;

	Ref< Heightfield > heightfield = new Heightfield(
		size,
		worldExtent,
		worldCenter
	);

	height_t* heights = heightfield->getHeights();
	T_ASSERT_M (heights, L"No heights in heightfield");
	Reader(stream).read(heights, size * size, sizeof(height_t));

	uint8_t* cuts = heightfield->getCuts();
	T_ASSERT_M (cuts, L"No cuts in heightfield");
	Reader(stream).read(cuts, (version >= 4) ? (size * size + 7) / 8 : size * size / 8, sizeof(uint8_t));

	if (version == 2)
	{
		// Attributes used to be a single grid of exclusive identifiers; migrate each
		// identifier into it's own density layer, at full density.
		AutoArrayPtr< uint8_t > identifiers(new uint8_t [size * size]);
		Reader(stream).read(identifiers.ptr(), size * size, sizeof(uint8_t));

		for (int32_t i = 0; i < size * size; ++i)
		{
			uint8_t* attributes = heightfield->createAttributes(identifiers[i]);
			if (attributes != nullptr)
				attributes[i] = 255;
		}
	}
	else if (version >= 3)
	{
		uint32_t mask;
		Reader(stream) >> mask;

		for (uint8_t i = 0; i < Heightfield::MaxAttributes; ++i)
		{
			if ((mask & (1 << i)) == 0)
				continue;

			uint8_t* attributes = heightfield->createAttributes(i);
			T_ASSERT_M (attributes, L"No attributes in heightfield");
			Reader(stream).read(attributes, size * size, sizeof(uint8_t));
		}
	}

	if ((layers & Heightfield::LySplat) != 0)
		Reader(stream).read(heightfield->createSplats(), (int64_t)size * size * 4, sizeof(uint8_t));

	if ((layers & Heightfield::LyColor) != 0)
		Reader(stream).read(heightfield->createColors(), (int64_t)size * size * 4, sizeof(uint8_t));

	heightfield->updateCellBounds();

	stream->close();
	return heightfield;
}

bool HeightfieldFormat::readHeader(IStream* stream, int32_t& outSize, uint32_t& outLayers) const
{
	int32_t version;
	Reader(stream) >> version;

	if (version < 1 || version > c_version)
		return false;

	Reader(stream) >> outSize;

	outLayers = 0;
	if (version >= 5)
		Reader(stream) >> outLayers;

	return true;
}

bool HeightfieldFormat::write(IStream* stream, const Heightfield* heightfield) const
{
	// Optional layers are part of header so they can be queried without reading entire heightfield.
	uint32_t layers = 0;
	if (heightfield->getSplats())
		layers |= Heightfield::LySplat;
	if (heightfield->getColors())
		layers |= Heightfield::LyColor;

	Writer(stream) << int32_t(c_version);
	Writer(stream) << int32_t(heightfield->getSize());
	Writer(stream) << layers;

	Writer(stream).write(
		heightfield->getHeights(),
		heightfield->getSize() * heightfield->getSize(),
		sizeof(height_t)
	);

	Writer(stream).write(
		heightfield->getCuts(),
		(heightfield->getSize() * heightfield->getSize() + 7) / 8,
		sizeof(uint8_t)
	);

	// Only layers which carry any density are written.
	uint32_t mask = 0;
	for (uint8_t i = 0; i < Heightfield::MaxAttributes; ++i)
	{
		if (heightfield->haveAttribute(i))
			mask |= 1 << i;
	}

	Writer(stream) << mask;

	for (uint8_t i = 0; i < Heightfield::MaxAttributes; ++i)
	{
		if ((mask & (1 << i)) == 0)
			continue;

		Writer(stream).write(
			heightfield->getAttributes(i),
			heightfield->getSize() * heightfield->getSize(),
			sizeof(uint8_t)
		);
	}

	if ((layers & Heightfield::LySplat) != 0)
		Writer(stream).write(heightfield->getSplats(), (int64_t)heightfield->getSize() * heightfield->getSize() * 4, sizeof(uint8_t));

	if ((layers & Heightfield::LyColor) != 0)
		Writer(stream).write(heightfield->getColors(), (int64_t)heightfield->getSize() * heightfield->getSize() * 4, sizeof(uint8_t));

	return true;
}

}
