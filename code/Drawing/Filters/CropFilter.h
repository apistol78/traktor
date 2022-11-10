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

/*! Crop filter.
 * \ingroup Drawing
 */
class T_DLLCLASS CropFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	enum AnchorType
	{
		AtLeft = -1,
		AtUp = -1,
		AtCenter = 0,
		AtRight = 1,
		AtDown = 1
	};

	explicit CropFilter(
		AnchorType anchorX,
		AnchorType anchorY,
		int32_t width,
		int32_t height
	);

protected:
	virtual void apply(Image* image) const override final;

private:
	AnchorType m_anchorX;
	AnchorType m_anchorY;
	int32_t m_width;
	int32_t m_height;
};

}
