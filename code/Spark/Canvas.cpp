/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Bitmap.h"
#include "Spark/Canvas.h"
#include "Spark/Triangulator.h"
#include "Spark/Types.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Canvas", Canvas, Object)

Canvas::Canvas()
:	m_cacheTag(allocateCacheTag())
,	m_dirtyTag(0)
,	m_drawing(false)
{
	clear();
}

int32_t Canvas::getCacheTag() const
{
	return m_cacheTag;
}

int32_t Canvas::getDirtyTag() const
{
	return m_dirtyTag;
}

void Canvas::clear()
{
	m_dictionary = Dictionary();
	m_paths.clear();
	m_bounds = Aabb2();
	m_fillStyles.resize(0);
	m_lineStyles.resize(0);
	++m_dirtyTag;
}

void Canvas::beginFill(const Color4f& color)
{
	FillStyle style;
	style.create(color);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::beginGradientFill(FillStyle::GradientType gradientType, const AlignedVector< FillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix)
{
	FillStyle style;
	style.create(gradientType, colorRecords, gradientMatrix);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::beginBitmapFill(Bitmap* image, const Matrix33& bitmapMatrix, bool repeat)
{
	const uint16_t bitmapId = m_dictionary.addBitmap(image);

	FillStyle style;
	style.create(bitmapId, bitmapMatrix, repeat);

	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::endFill()
{
	if (!m_drawing)
		return;

	const uint16_t fillStyle = uint16_t(m_fillStyles.size());

	Path& p = m_paths.back();
	p.end(fillStyle, 0, 0);

	m_drawing = false;
	++m_dirtyTag;
}

void Canvas::moveTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.moveTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void Canvas::lineTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.lineTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void Canvas::curveTo(float controlX, float controlY, float anchorX, float anchorY)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.quadraticTo(int32_t(controlX), int32_t(controlY), int32_t(anchorX), int32_t(anchorY), Path::CmAbsolute);
	m_bounds.contain(Vector2(controlX, controlY));
	m_bounds.contain(Vector2(anchorX, anchorY));
}

void Canvas::triangulate(bool oddEven, AlignedVector< Triangle >& outTriangles, AlignedVector< Line >& outLines) const
{
	AlignedVector< Segment > segments;
	Triangulator triangulator;
	Segment s;

	outTriangles.resize(0);
	outLines.resize(0);

	for (const auto& path : m_paths)
	{
		const AlignedVector< Vector2 >& points = path.getPoints();
		const AlignedVector< SubPath >& subPaths = path.getSubPaths();

		SmallSet< uint16_t > fillStyles;
		for (const auto& sp : subPaths)
		{
			if (sp.fillStyle0)
				fillStyles.insert(sp.fillStyle0);
			if (sp.fillStyle1)
				fillStyles.insert(sp.fillStyle1);
		}

		for (const auto fillStyle : fillStyles)
		{
			for (const auto& sp : subPaths)
			{
				if (sp.fillStyle0 != fillStyle && sp.fillStyle1 != fillStyle)
					continue;

				for (const auto& segment : sp.segments)
				{
					switch (segment.type)
					{
					case SpgtLinear:
						{
							s.v[0] = points[segment.pointsOffset];
							s.v[1] = points[segment.pointsOffset + 1];
							s.curve = false;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					case SpgtQuadratic:
						{
							s.v[0] = points[segment.pointsOffset];
							s.v[1] = points[segment.pointsOffset + 2];
							s.c = points[segment.pointsOffset + 1];
							s.curve = true;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					default:
						break;
					}
				}
			}

			if (!segments.empty())
			{
				const uint32_t from = (uint32_t)outTriangles.size();

				triangulator.triangulate(segments, fillStyle, oddEven, outTriangles);
				segments.resize(0);

				const uint32_t to = (uint32_t)outTriangles.size();

				// Transform each new triangle with path's transform.
				for (uint32_t ti = from; ti < to; ++ti)
				{
					outTriangles[ti].v[0] = path.getTransform() * outTriangles[ti].v[0];
					outTriangles[ti].v[1] = path.getTransform() * outTriangles[ti].v[1];
					outTriangles[ti].v[2] = path.getTransform() * outTriangles[ti].v[2];
				}
			}
		}
	}
}

}
