#include <algorithm>
#include "Flash/FlashFont.h"
#include "Flash/FlashShape.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashFont", FlashFont, ActionObject)

FlashFont::FlashFont()
:	ActionObject(AsObject::getInstance())
,	m_ascent(0)
,	m_descent(0)
,	m_leading(0)
,	m_coordinateType(CtTwips)
{
}

bool FlashFont::create(const AlignedVector< SwfShape* >& shapeTable)
{
	m_shapes.resize(shapeTable.size());
	for (uint32_t i = 0; i < shapeTable.size(); ++i)
	{
		m_shapes[i] = gc_new< FlashShape >(0);
		if (!m_shapes[i]->create(shapeTable[i]))
			return false;
	}
	m_coordinateType = CtTwips;
	return true;
}

bool FlashFont::create(
	const AlignedVector< SwfShape* >& shapeTable,
	int16_t ascent,
	int16_t descent,
	int16_t leading,
	const AlignedVector< int16_t >& advanceTable,
	const AlignedVector< SwfRect >& boundsTable,
	const AlignedVector< SwfKerningRecord >& kerningRecords,
	const AlignedVector< uint16_t >& codeTable,
	CoordinateType coordinateType
)
{
	if (!create(shapeTable))
		return false;

	m_ascent = ascent;
	m_descent = descent;
	m_leading = leading;
	m_advanceTable = advanceTable;
	m_boundsTable = boundsTable;
	m_kerningRecords = kerningRecords;
	m_codeTable = codeTable;
	m_coordinateType = coordinateType;

	return true;
}

const FlashShape* FlashFont::getShape(uint16_t index) const
{
	return index < m_shapes.size() ? m_shapes[index] : 0;
}

int16_t FlashFont::getAscent() const
{
	return m_ascent;
}

int16_t FlashFont::getDescent() const
{
	return m_descent;
}

int16_t FlashFont::getLeading() const
{
	return m_leading;
}

int16_t FlashFont::getAdvance(uint16_t index) const
{
	return index < m_advanceTable.size() ? m_advanceTable[index] : 0;
}

const SwfRect* FlashFont::getBounds(uint16_t index) const
{
	return index < m_boundsTable.size() ? &m_boundsTable[index] : 0;
}

const SwfKerningRecord* FlashFont::lookupKerningRecord(uint16_t leftCode, uint16_t rightCode) const
{
	for (AlignedVector< SwfKerningRecord >::const_iterator i = m_kerningRecords.begin(); i != m_kerningRecords.end(); ++i)
	{
		if (i->code1 == leftCode && i->code2 == rightCode)
			return &(*i);
	}
	return 0;
}

uint16_t FlashFont::lookupIndex(uint16_t code) const
{
	AlignedVector< uint16_t >::const_iterator i = std::find(m_codeTable.begin(), m_codeTable.end(), code);
	if (i == m_codeTable.end())
		return 0;

	size_t distance = std::distance(m_codeTable.begin(), i);
	T_ASSERT (distance < m_shapes.size());

	return uint16_t(distance);
}

FlashFont::CoordinateType FlashFont::getCoordinateType() const
{
	return m_coordinateType;
}

	}
}
