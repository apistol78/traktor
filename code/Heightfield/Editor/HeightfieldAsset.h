/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class HeightfieldAssetLayer;

/*! Heightfield asset.
 * \ingroup Heightfield
 *
 * Heightfield asset is a database instance which contain
 * meta information about heightfield.
 * Layers are evaluated bottom first; data of edit layers, and
 * masks of erosion layers, is stored as database instance
 * data channels, a complete set of channels for each layer.
 */
class T_DLLCLASS HeightfieldAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldAsset();

	explicit HeightfieldAsset(const Vector4& worldExtent);

	virtual void serialize(ISerializer& s) override final;

	void setWorldExtent(const Vector4& worldExtent) { m_worldExtent = worldExtent; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	void setTileCount(int32_t tileCount) { m_tileCount = tileCount; }

	/*! Number of tiles along each side; 0 if data is stored as a single, untiled, channel. */
	int32_t getTileCount() const { return m_tileCount; }

	//! Add layer on top, with an unused identifier.
	void addLayer(HeightfieldAssetLayer* layer);

	//! Layers, bottom first; bottom layer is always an edit layer.
	RefArray< HeightfieldAssetLayer >& getLayers() { return m_layers; }

	//! Layers, bottom first; bottom layer is always an edit layer.
	const RefArray< HeightfieldAssetLayer >& getLayers() const { return m_layers; }

private:
	Vector4 m_worldExtent = Vector4::zero();
	int32_t m_tileCount = 0;
	int32_t m_nextLayerId = 1;
	RefArray< HeightfieldAssetLayer > m_layers;
};

}
