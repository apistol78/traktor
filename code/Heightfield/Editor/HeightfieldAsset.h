/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Heightfield asset.
 * \ingroup Heightfield
 *
 * Heightfield asset is a database instance which contain
 * meta information about heightfield.
 * Actual heightfield data is stored as database instance
 * data channels.
 */
class T_DLLCLASS HeightfieldAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldAsset() = default;

	explicit HeightfieldAsset(const Vector4& worldExtent);

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	bool getErosionEnable() const { return m_erosionEnable; }

	int32_t getErodeIterations() const { return m_erodeIterations; }

private:
	Vector4 m_worldExtent = Vector4::zero();
	bool m_erosionEnable = 0.0f;
	int32_t m_erodeIterations = 100000;
};

}

