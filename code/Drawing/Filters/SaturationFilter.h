/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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

/*! Saturation filter.
 * \ingroup Drawing
 */
class T_DLLCLASS SaturationFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit SaturationFilter(float saturation);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_saturation;
};

}
