/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

/*! Compute texture asset.
 * \ingroup Render
 *
 * Serializable descriptor for a compute-driven texture.
 * References a compute shader and describes the output texture
 * format and dimensions.
 */
class T_DLLCLASS ComputeTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	TextureFormat getFormat() const { return m_format; }

	bool isContinuous() const { return m_continuous; }

private:
	resource::Id< Shader > m_shader;
	int32_t m_width = 256;
	int32_t m_height = 256;
	TextureFormat m_format = TfR8G8B8A8;
	bool m_continuous = true;	//!< If true, compute shader is dispatched every frame.
};

}
