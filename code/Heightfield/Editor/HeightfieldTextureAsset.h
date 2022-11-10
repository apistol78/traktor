/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;

class T_DLLCLASS HeightfieldTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum OutputType
	{
		OtHeights,
		OtNormals,
		OtCuts,
		OtUnwrap
	};

	resource::Id< Heightfield > m_heightfield;
	OutputType m_output;
	float m_scale;

	HeightfieldTextureAsset();

	virtual void serialize(ISerializer& s) override final;
};

	}
}

