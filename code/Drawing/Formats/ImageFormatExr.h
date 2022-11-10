/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_EXR)

namespace traktor::drawing
{

/*! EXR format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatExr : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) override final;

	virtual bool write(IStream* stream, const Image* image) override final;
};

}

#endif	// DRAWING_INCLUDE_EXR
