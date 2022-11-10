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

/*! Brightness & Contrast filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BrightnessContrastFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit BrightnessContrastFilter(float brightness, float contrast);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_brightness;
	float m_contrast;
};

}
