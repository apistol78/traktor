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
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS TextureOutput : public ISerializable
{
	T_RTTI_CLASS;

public:
	TextureFormat m_textureFormat = TfInvalid;
	bool m_normalMap = false;
	float m_scaleDepth = 0.0f;
	bool m_generateMips = true;
	bool m_keepZeroAlpha = false;
	TextureType m_textureType = Tt2D;
	bool m_hasAlpha = false;
	bool m_generateAlpha = false;
	bool m_ignoreAlpha = false;
	bool m_invertAlpha = false;
	bool m_premultiplyAlpha = false;
	bool m_dilateImage = false;
	bool m_scaleImage = false;
	int32_t m_scaleWidth = 0;
	int32_t m_scaleHeight = 0;
	bool m_flipX = false;
	bool m_flipY = false;
	bool m_enableCompression = true;
	bool m_encodeAsRGBM = false;
	bool m_inverseNormalMapX = false;
	bool m_inverseNormalMapY = false;
	float m_scaleNormalMap = 0.0f;
	bool m_assumeLinearGamma = false;
	bool m_generateSphereMap = false;
	bool m_preserveAlphaCoverage = false;
	float m_alphaCoverageReference = 0.5f;
	int32_t m_sharpenRadius = 0;
	float m_sharpenStrength = 0.0f;
	float m_noiseStrength = 0.0f;
	bool m_systemTexture = false;

	virtual void serialize(ISerializer& s) override;
};

}
