/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Blur filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BlurFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit BlurFilter(int32_t x, int32_t y);

protected:
	virtual void apply(Image* image) const override final;

private:
	int32_t m_x;
	int32_t m_y;
};

}
