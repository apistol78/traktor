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

/*! RGBM encoding filter.
 * \ingroup Drawing
 */
class T_DLLCLASS EncodeRGBM : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit EncodeRGBM(float range, int32_t blockSizeX = 0, int32_t blockSizeY = 0, float blockFilterRatio = 0.0f);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_range;
	int32_t m_blockSizeX;
	int32_t m_blockSizeY;
	float m_blockFilterRatio;
};

}
