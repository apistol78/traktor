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

namespace traktor::hf
{

class Heightfield;

class T_DLLCLASS OcclusionTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class OcclusionTexturePipeline;

	resource::Id< Heightfield > m_heightfield;
	resource::Id< ISerializable > m_occluderData;
	uint32_t m_size = 1024;
	float m_traceDistance = 16.0f;
	int32_t m_blurRadius = 0;
};

}
