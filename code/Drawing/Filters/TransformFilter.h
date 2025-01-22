/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Transform colors filter.
 * \ingroup Drawing
 *
 * Linearly transform colors; RGBA' = RGBA * Km + Kc
 */
class T_DLLCLASS TransformFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit TransformFilter(const Color4f& Km, const Color4f& Kc);

protected:
	virtual void apply(Image* image) const override final;

private:
	Color4f m_Km;
	Color4f m_Kc;
};

}
