/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Texture built from one layer of a trim sheet setup.
 * \ingroup Render
 *
 * The layer determine how the texture is built; albedo is sRGB color,
 * normal is a tangent space normal map, height is a single channel
 * linear map, roughness is linear single channel data (exact only if
 * compression is disabled) and specular is linear color.
 */
class T_DLLCLASS TrimSheetTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setSetup(const Guid& setup) { m_setup = setup; }

	const Guid& getSetup() const { return m_setup; }

	void setLayer(TrimSheetLayer layer) { m_layer = layer; }

	TrimSheetLayer getLayer() const { return m_layer; }

	/*! Keep alpha channel; only applicable to albedo and specular. */
	bool getKeepAlpha() const { return m_keepAlpha; }

	bool getGenerateMips() const { return m_generateMips; }

	bool getEnableCompression() const { return m_enableCompression; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_setup;
	TrimSheetLayer m_layer = TrimSheetLayer::Albedo;
	bool m_keepAlpha = false;
	bool m_generateMips = true;
	bool m_enableCompression = true;
};

}
