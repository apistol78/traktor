/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;
class HeightfieldAsset;
class TileStorage;

/*! Layer of heightfield asset.
 * \ingroup Heightfield
 *
 * Layers are evaluated bottom first, each layer
 * modify output of layer below it.
 */
class T_DLLCLASS HeightfieldAssetLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Apply layer onto tile.
	 *
	 * \param storage Storage of heightfield's layers.
	 * \param layer Index of this layer in storage.
	 * \param worldExtent World extent of entire heightfield.
	 * \param inoutTile Tile of layers below merged, positioned in world.
	 * \return False if layer failed to read its data.
	 */
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const = 0;

	/*! Read tile of layer alone, positioned in world.
	 *
	 * \param outMask Optional mask of tile, one byte per sample, where layer modify layers below.
	 */
	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask) const = 0;

	//! True if each tile of layer depend on entire heightfield of layers below.
	virtual bool isGlobal() const;

	//! True if all channels of layer's tiles are stored.
	virtual bool haveData() const;

	//! True if mask of layer's tiles is stored.
	virtual bool haveMask() const;

	virtual void serialize(ISerializer& s) override;

	//! Unique identifier, names layer's data channels; never reused.
	int32_t getId() const { return m_id; }

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

private:
	friend class HeightfieldAsset;

	int32_t m_id = 0;
	std::wstring m_name;
};

}
