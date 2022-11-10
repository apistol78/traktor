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

/*! Mirror image filter.
 * \ingroup Drawing
 *
 * Flip image horizontally and/or vertically.
 */
class T_DLLCLASS MirrorFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit MirrorFilter(bool flipHorizontal, bool flipVertical);

protected:
	virtual void apply(Image* image) const override final;

private:
	bool m_flipHorizontal;
	bool m_flipVertical;
};

}
