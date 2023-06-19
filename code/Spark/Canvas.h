/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Spark/Dictionary.h"
#include "Spark/FillStyle.h"
#include "Spark/LineStyle.h"
#include "Spark/Path.h"
#include "Spark/Polygon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Bitmap;

/*! Dynamic canvas.
 * \ingroup Spark
 */
#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif
class T_DLLCLASS Canvas : public Object
{
	T_RTTI_CLASS;

public:
	Canvas();

	int32_t getCacheTag() const;

	int32_t getDirtyTag() const;

	/*! Clear canvas. */
	void clear();

	/*! Begin solid fill. */
	void beginFill(const Color4f& color);

	/*! Begin gradient fill. */
	void beginGradientFill(FillStyle::GradientType gradientType, const AlignedVector< FillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix);

	/*! Begin bitmap fill. */
	void beginBitmapFill(Bitmap* image, const Matrix33& bitmapMatrix, bool repeat);

	/*! End fill. */
	void endFill();

	/*! Move cursor. */
	void moveTo(float x, float y);

	/*! Add a line from cursor. */
	void lineTo(float x, float y);

	/*! Add a curve from cursor. */
	void curveTo(float controlX, float controlY, float anchorX, float anchorY);

	/*! Generate triangles and lines from canvas. */
	void triangulate(bool oddEven, AlignedVector< Triangle >& outTriangles, AlignedVector< Line >& outLines) const;

	/*! Get bounds of shapes in canvas. */
	const Aabb2& getBounds() const { return m_bounds; }

	const Dictionary& getDictionary() const { return m_dictionary; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

private:
	int32_t m_cacheTag;
	int32_t m_dirtyTag;
	Aabb2 m_bounds;
	Dictionary m_dictionary;
	AlignedVector< Path > m_paths;
	AlignedVector< LineStyle > m_lineStyles;
	AlignedVector< FillStyle > m_fillStyles;
	bool m_drawing;
};

}
