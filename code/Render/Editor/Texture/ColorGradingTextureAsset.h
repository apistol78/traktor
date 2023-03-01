/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS ColorGradingTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	ColorGradingTextureAsset();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ColorGradingAssetEditor;
	friend class ColorGradingTexturePipeline;

	AlignedVector< std::pair< float, float > > m_redCurve;
	AlignedVector< std::pair< float, float > > m_greenCurve;
	AlignedVector< std::pair< float, float > > m_blueCurve;

	float m_brightness = 0.0f;
	float m_contrast = 1.0f;
	float m_saturation = 1.0f;
};

}
