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
	/*! Rasterization options.
	 *
	 * Defaults reproduce the behaviour of the simple generate overload; only
	 * front facing polygons are drawn and every covered pixel become opaque.
	 */
	struct Options
	{
		/*! Discard fragments whose source alpha is below this threshold; zero disables the test.
		 *
		 * Source alpha is only taken from the material when it is flagged as alpha
		 * blended or alpha tested, since opaque materials commonly carry unrelated
		 * data in the alpha channel of their diffuse map.
		 */
		float alphaThreshold = 0.0f;

		/*! Write source alpha into the target instead of forcing covered pixels opaque. */
		bool writeAlpha = false;

		/*! Rasterize back facing polygons as well. */
		bool twoSided = false;

		/*! Write flat albedo instead of shading with the built in sun.
		 *
		 * Colors are then written exactly as the material carry them, which is what a
		 * consumer doing its own lighting want; the shaded path is only meant for
		 * previews.
		 */
		bool unlit = false;
	};

	/*! Rasterize model through a 70 degree perspective projection. */
	bool generate(const Model* model, const Matrix44& modelView, drawing::Image* outImage) const;

	/*! Rasterize model through an explicit projection.
	 *
	 * The target images are never cleared; fragments are written only where the model
	 * is visible so the caller decide the background.
	 *
	 * \param outNormalImage Optional target receiving the shading normal of each
	 *        fragment, in the space the model view put it in, encoded as n * 0.5 + 0.5.
	 *        Must be the same size as outImage.
	 */
	bool generate(const Model* model, const Matrix44& modelView, const Matrix44& projection, const Options& options, drawing::Image* outImage, drawing::Image* outNormalImage = nullptr) const;
};

}
