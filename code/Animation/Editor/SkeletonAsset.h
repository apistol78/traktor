/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS SkeletonAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	SkeletonAsset();

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getOffset() const { return m_offset; }

	float getScale() const { return m_scale; }

	float getRadius() const { return m_radius; }

	bool getInvertX() const { return m_invertX; }

	bool getInvertZ() const { return m_invertZ; }

private:
	Vector4 m_offset;
	float m_scale;
	float m_radius;
	bool m_invertX;
	bool m_invertZ;
};

	}
}
