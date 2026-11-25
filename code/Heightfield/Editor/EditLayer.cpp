/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/EditLayer.h"

#include "Core/Log/Log.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

namespace traktor::hf
{
namespace
{

//! Blend values of layer onto merged by mask.
void blendValues(uint8_t* merged, const uint8_t* layer, const uint8_t* mask, int32_t samples, int32_t components)
{
	for (int32_t i = 0; i < samples; ++i)
	{
		const uint32_t m = mask[i];
		if (m == 0)
			continue;

		for (int32_t c = 0; c < components; ++c)
		{
			const size_t j = (size_t)i * components + c;
			merged[j] = (uint8_t)((merged[j] * (255 - m) + layer[j] * m + 127) / 255);
		}
	}
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.EditLayer", 0, EditLayer, HeightfieldAssetLayer)

bool EditLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	const int32_t size = inoutTile->getSize();
	const int32_t samples = size * size;

	// Tile without mask doesn't contribute.
	AlignedVector< uint8_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
		return true;

	Ref< const Heightfield > tile = storage->readStoredTile(layer, worldExtent, tileX, tileZ);
	if (!tile || tile->getSize() != size)
	{
		log::error << L"Unable to read heightfield; tile " << tileX << L", " << tileZ << L" of layer " << layer << L" missing or size mismatch." << Endl;
		return false;
	}

	// Optional channels and attributes which layer doesn't have are kept.
	height_t* heights = inoutTile->getHeights();
	const height_t* layerHeights = tile->getHeights();
	for (int32_t i = 0; i < samples; ++i)
	{
		const uint32_t m = mask[i];
		if (m == 0)
			continue;

		heights[i] = (height_t)((heights[i] * (255 - m) + layerHeights[i] * m + 127) / 255);
		if (m >= 128)
			inoutTile->setGridCut(i % size, i / size, tile->getGridCut(i % size, i / size));
	}

	for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
		if (const uint8_t* attributes = tile->getAttributes(a))
			blendValues(inoutTile->createAttributes(a), attributes, mask.c_ptr(), samples, 1);

	if (const uint8_t* splats = tile->getSplats())
		blendValues(inoutTile->createSplats(), splats, mask.c_ptr(), samples, 4);

	if (const uint8_t* colors = tile->getColors())
		blendValues(inoutTile->createColors(), colors, mask.c_ptr(), samples, 4);

	return true;
}

Ref< Heightfield > EditLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask) const
{
	// Tiles above base without data are empty.
	Ref< Heightfield > tile = storage->readStoredTile(layer, worldExtent, tileX, tileZ);
	if (!tile && layer > 0)
		tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
	{
		log::error << L"Unable to read heightfield; no tile " << tileX << L", " << tileZ << L" of layer " << layer << L" in instance." << Endl;
		return nullptr;
	}

	// Base layer cover everything, layers above nothing unless masked.
	if (outMask)
	{
		const int32_t size = tile->getSize();
		if (layer == 0 || !storage->readStoredMask(layer, tileX, tileZ, size, *outMask))
		{
			outMask->resize(0);
			outMask->resize((size_t)size * size, (layer == 0) ? 255 : 0);
		}
	}

	return tile;
}

bool EditLayer::haveData() const
{
	return true;
}

bool EditLayer::haveMask() const
{
	return true;
}

}
