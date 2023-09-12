/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Shape;

/*! Font
 * \ingroup Spark
 */
class T_DLLCLASS Font : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum CoordinateType
	{
		CtTwips,
		CtEMSquare
	};

	bool initializeFromShapes(const RefArray< Shape >& shapeTable);

	bool initialize(
		const std::string& fontName,
		bool italic,
		bool bold,
		const RefArray< Shape >& shapeTable,
		int16_t ascent,
		int16_t descent,
		int16_t leading,
		const AlignedVector< int16_t >& advanceTable,
		const AlignedVector< Aabb2 >& boundsTable,
		const SmallMap< uint32_t, int16_t >& kerningLookup,
		const AlignedVector< uint16_t >& codeTable,
		CoordinateType coordinateType
	);

	const std::string& getFontName() const;

	bool isItalic() const;

	bool isBold() const;

	const RefArray< Shape >& getShapes() const;

	const Shape* getShape(uint16_t index) const;

	int16_t getAscent() const;

	int16_t getDescent() const;

	int16_t getLeading() const;

	int16_t getAdvance(uint16_t index) const;

	const Aabb2* getBounds(uint16_t index) const;

	const Vector2& getMaxDimension() const;

	int16_t lookupKerning(uint16_t leftCode, uint16_t rightCode) const;

	uint16_t lookupIndex(uint16_t code) const;

	CoordinateType getCoordinateType() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_fontName;
	bool m_italic = false;
	bool m_bold = false;
	RefArray< Shape > m_shapes;
	int16_t m_ascent = 0;
	int16_t m_descent = 0;
	int16_t m_leading = 0;
	AlignedVector< int16_t > m_advanceTable;
	AlignedVector< Aabb2 > m_boundsTable;
	Vector2 m_maxDimension = Vector2::zero();
	SmallMap< uint32_t, int16_t > m_kerningLookup;
	SmallMap< uint16_t, uint16_t > m_indexLookup;
	CoordinateType m_coordinateType = CtTwips;
};

}
