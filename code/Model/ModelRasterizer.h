/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix44;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::model
{

class Model;

/*! Simple model software rasterizer.
 * \ingroup Model
 */
class T_DLLCLASS ModelRasterizer : public Object
{
	T_RTTI_CLASS;

public:
	bool generate(const Model* model, const Matrix44& modelView, drawing::Image* outImage) const;
};

}
