/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Flash/Font.h"
#include "Flash/Shape.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Font", 0, Font, ISerializable)

Font::Font()
:	m_italic(false)
,	m_bold(false)
,	m_ascent(0)
,	m_descent(0)
,	m_leading(0)
,	m_maxDimension(0.0f, 0.0f)
,	m_coordinateType(CtTwips)
{
}

bool Font::create(const AlignedVector< SwfShape* >& shapeTable)
{
	m_shapes.resize(shapeTable.size());
	for (uint32_t i = 0; i < shapeTable.size(); ++i)
	{
		m_shapes[i] = new Shape(0);
		if (!m_shapes[i]->create(shapeTable[i]))
			return false;

		Aabb2 shapeBounds = m_shapes[i]->getShapeBounds();
		m_maxDimension = max(m_maxDimension, shapeBounds.mx - shapeBounds.mn);
	}
	m_coordinateType = CtTwips;
	return true;
}

bool Font::create(
	const std::string& fontName,
	bool italic,
	bool bold,
	const AlignedVector< SwfShape* >& shapeTable,
	int16_t ascent,
	int16_t descent,
	int16_t leading,
	const AlignedVector< int16_t >& advanceTable,
	const AlignedVector< Aabb2 >& boundsTable,
	const AlignedVector< SwfKerningRecord >& kerningRecords,
	const AlignedVector< uint16_t >& codeTable,
	CoordinateType coordinateType
)
{
	if (!create(shapeTable))
		return false;

	m_fontName = fontName;
	m_italic = italic;
	m_bold = bold;
	m_ascent = ascent;
	m_descent = descent;
	m_leading = leading;
	m_advanceTable = advanceTable;
	m_boundsTable = boundsTable;
	m_coordinateType = coordinateType;
	
	for (AlignedVector< SwfKerningRecord >::const_iterator i = kerningRecords.begin(); i != kerningRecords.end(); ++i)
	{
		uint32_t codePair = (uint32_t(i->code1) << 16) | i->code2;
		m_kerningLookup[codePair] = i->adjustment;
	}

	for (uint32_t i = 0; i < codeTable.size(); ++i)
		m_indexLookup[codeTable[i]] = i;

	for (AlignedVector< Aabb2 >::const_iterator i = boundsTable.begin(); i != boundsTable.end(); ++i)
		m_maxDimension = max(m_maxDimension, i->mx - i->mn);

	return true;
}

const std::string& Font::getFontName() const
{
	return m_fontName;
}

bool Font::isItalic() const
{
	return m_italic;
}

bool Font::isBold() const
{
	return m_bold;
}

const RefArray< Shape >& Font::getShapes() const
{
	return m_shapes;
}

const Shape* Font::getShape(uint16_t index) const
{
	return index < m_shapes.size() ? m_shapes[index] : 0;
}

int16_t Font::getAscent() const
{
	return m_ascent;
}

int16_t Font::getDescent() const
{
	return m_descent;
}

int16_t Font::getLeading() const
{
	return m_leading;
}

int16_t Font::getAdvance(uint16_t index) const
{
	return index < m_advanceTable.size() ? m_advanceTable[index] : 0;
}

const Aabb2* Font::getBounds(uint16_t index) const
{
	return index < m_boundsTable.size() ? &m_boundsTable[index] : 0;
}

const Vector2& Font::getMaxDimension() const
{
	return m_maxDimension;
}

int16_t Font::lookupKerning(uint16_t leftCode, uint16_t rightCode) const
{
	uint32_t codePair = (uint32_t(leftCode) << 16) | rightCode;
	SmallMap< uint32_t, int16_t >::const_iterator i = m_kerningLookup.find(codePair);
	return i != m_kerningLookup.end() ? i->second : 0;
}

uint16_t Font::lookupIndex(uint16_t code) const
{
	SmallMap< uint16_t, uint16_t >::const_iterator i = m_indexLookup.find(code);
	return i != m_indexLookup.end() ? i->second : 0;
}

Font::CoordinateType Font::getCoordinateType() const
{
	return m_coordinateType;
}

void Font::serialize(ISerializer& s)
{
	const MemberEnum< CoordinateType >::Key kCoordinateType[] =
	{
		{ L"CtTwips", CtTwips },
		{ L"CtEMSquare", CtEMSquare },
		{ 0, 0 }
	};

	s >> Member< std::string >(L"fontName", m_fontName);
	s >> Member< bool >(L"italic", m_italic);
	s >> Member< bool >(L"bold", m_bold);
	s >> MemberRefArray< Shape >(L"shapes", m_shapes);
	s >> Member< int16_t >(L"ascent", m_ascent);
	s >> Member< int16_t >(L"descent", m_descent);
	s >> Member< int16_t >(L"leading", m_leading);
	s >> MemberAlignedVector< int16_t >(L"advanceTable", m_advanceTable);
	s >> MemberAlignedVector< Aabb2, MemberAabb2 >(L"boundsTable", m_boundsTable);
	s >> Member< Vector2 >(L"maxDimension", m_maxDimension);
	s >> MemberSmallMap< uint32_t, int16_t >(L"kerningLookup", m_kerningLookup);
	s >> MemberSmallMap< uint16_t, uint16_t >(L"indexLookup", m_indexLookup);
	s >> MemberEnum< CoordinateType >(L"coordinateType", m_coordinateType, kCoordinateType);
}

	}
}
