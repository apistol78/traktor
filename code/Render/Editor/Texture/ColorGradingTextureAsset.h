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
	float getRedGamma() const { return m_redGamma; }

	float getGreenGamma() const { return m_greenGamma; }

	float getBlueGamma() const { return m_blueGamma; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_redGamma = 1.0f;
	float m_greenGamma = 1.0f;
	float m_blueGamma = 1.0f;
};

}
