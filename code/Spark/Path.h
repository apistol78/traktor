/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Vector2i.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace spark
	{

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

/*! Segment type.
 * \ingroup Spark
 */
enum SubPathSegmentType
{
	SpgtUndefined = 0,
	SpgtLinear = 1,
	SpgtQuadratic = 2
};

/*! Sub path segment.
 * \ingroup Spark
 */
struct SubPathSegment
{
	SubPathSegmentType type = SpgtUndefined;
	uint32_t pointsOffset = 0;
	uint32_t pointsCount = 0;

	SubPathSegment() = default;

	explicit SubPathSegment(SubPathSegmentType type_)
	:	type(type_)
	{
	}

	void serialize(ISerializer& s);
};

/*! Sub path.
 * \ingroup Spark
 */
struct SubPath
{
	uint16_t fillStyle0 = 0;
	uint16_t fillStyle1 = 0;
	uint16_t lineStyle = 0;
	AlignedVector< SubPathSegment > segments;

	void serialize(ISerializer& s);
};

/*! Shape path.
 * \ingroup Spark
 */
class T_DLLCLASS Path
{
public:
	enum CoordinateMode
	{
		CmRelative,
		CmAbsolute
	};

	Path();

	explicit Path(const Matrix33& transform, const AlignedVector< Vector2 >& points, const AlignedVector< SubPath >& subPaths);

	/*! Reset path. */
	void reset();

	/*! Move cursor to position.
	 *
	 * \param x Cursor x position.
	 * \param y Cursor y position.
	 * \param mode Coordinate mode.
	 */
	void moveTo(int32_t x, int32_t y, CoordinateMode mode);

	/*! Line from cursor to position.
	 *
	 * \param x End x position.
	 * \param y End y position.
	 * \param mode Coordinate mode.
	 */
	void lineTo(int32_t x, int32_t y, CoordinateMode mode);

	/*! Quadratic spline from cursor to position.
	 *
	 * \param x1 Control point.
	 * \param y1 Control point.
	 * \param x End x position.
	 * \param y End y position.
	 * \param mode Coordinate mode.
	 */
	void quadraticTo(int32_t x1, int32_t y1, int32_t x, int32_t y, CoordinateMode mode);

	/*! End path.
	 *
	 * \param fillStyle0 Index to odd fill style, 0 = no style.
	 * \param fillStyle1 Index to even fill style, 0 = no style.
	 * \param lineStyle Index to line style, 0 = no style.
	 */
	void end(uint16_t fillStyle0, uint16_t fillStyle1, uint16_t lineStyle);

	/*! Get origin of current sub path.
	 */
	Vector2 getOrigin() const;

	/*! Get bounds.
	 */
	Aabb2 getBounds() const;

	/*! Get cursor position.
	 *
	 * \return Cursor position.
	 */
	const Vector2& getCursor() const { return m_cursor; }

	/*! Get path local transform. */
	const Matrix33& getTransform() const { return m_transform; }

	/*! Get points.
	 *
	 * \return List of points.
	 */
	const AlignedVector< Vector2 >& getPoints() const { return m_points; }

	/*! Get sub paths.
	 *
	 * \return List of sub-paths.
	 */
	const AlignedVector< SubPath >& getSubPaths() const { return m_subPaths; }

	/*! Serialize path.
	 */
	void serialize(ISerializer& s);

private:
	Vector2 m_cursor;
	Matrix33 m_transform;
	AlignedVector< Vector2 > m_points;
	AlignedVector< SubPath > m_subPaths;
	SubPath m_current;

	/*! Transform between coordinate modes.
	 *
	 * \param from From coordinate mode.
	 * \param to To coordinate mode.
	 * \param x X coordinate.
	 * \param y Y coordinate.
	 */
	void transform(CoordinateMode from, CoordinateMode to, int32_t& x, int32_t& y) const;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

