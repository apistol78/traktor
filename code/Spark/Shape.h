/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/Path.h"
#include "Spark/Polygon.h"
#include "Spark/Character.h"
#include "Spark/FillStyle.h"
#include "Spark/LineStyle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

struct SwfShape;
struct SwfStyles;

/*! Shape
 * \ingroup Spark
 */
class T_DLLCLASS Shape : public Character
{
	T_RTTI_CLASS;

public:
	bool create(const Aabb2& shapeBounds, const SwfShape* shape, const SwfStyles* styles);

	bool create(const SwfShape* shape);

	bool create(uint16_t fillBitmap, int32_t width, int32_t height);

	void merge(const Shape& shape, const Matrix33& transform, const ColorTransform& cxform);

	void triangulate(bool oddEven, AlignedVector< Triangle >& outTriangles, AlignedVector< Line >& outLines) const;

	void triangulate(bool oddEven);

	void discardPaths();

	void addPath(const Path& path);

	uint16_t defineFillStyle(const Color4f& color);

	uint16_t defineFillStyle(uint16_t fillBitmap, const Matrix33& fillBitmapMatrix, bool fillBitmapRepeat);

	uint16_t defineLineStyle(const Color4f& color, uint16_t width);

	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< Triangle >& getTriangles() const { return m_triangles; }

	const AlignedVector< Line >& getLines() const { return m_lines; }

	virtual void serialize(ISerializer& s) override final;

private:
	Aabb2 m_shapeBounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FillStyle > m_fillStyles;
	AlignedVector< LineStyle > m_lineStyles;
	AlignedVector< Triangle > m_triangles;
	AlignedVector< Line > m_lines;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

