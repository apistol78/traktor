/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
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
	/*! Rasterization options; defaults reproduce the simple generate overload. */
	struct Options
	{
		/*! Discard fragments with alpha below threshold; alpha is only read from alpha blended or tested materials. */
		float alphaThreshold = 0.0f;

		/*! Write source alpha into the target instead of forcing covered pixels opaque. */
		bool writeAlpha = false;

		/*! Rasterize back facing polygons as well. */
		bool twoSided = false;

		/*! Write flat albedo instead of shading with the built in sun. */
		bool unlit = false;
	};

	/*! Rasterize model through a 70 degree perspective projection. */
	bool generate(const Model* model, const Matrix44& modelView, drawing::Image* outImage) const;

	/*! Rasterize model through an explicit projection; target images are not cleared.
	 *
	 * \param outNormalImage Optional target, same size as outImage, receiving view space normals encoded as n * 0.5 + 0.5.
	 */
	bool generate(const Model* model, const Matrix44& modelView, const Matrix44& projection, const Options& options, drawing::Image* outImage, drawing::Image* outNormalImage = nullptr) const;
};

}
