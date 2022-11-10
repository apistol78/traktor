/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Svg/Shape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace svg
	{

/*! SVG document.
 * \ingroup SVG
 */
class T_DLLCLASS Document : public Shape
{
	T_RTTI_CLASS;

public:
	Document();

	void setViewBox(const Aabb2& viewBox);

	const Aabb2& getViewBox() const;

	void setSize(const Vector2& size);

	const Vector2& getSize() const;

private:
	Aabb2 m_viewBox;
	Vector2 m_size;
};

	}
}

