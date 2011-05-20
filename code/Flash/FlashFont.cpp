#include "Flash/FlashFont.h"
#include "Flash/FlashShape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashFont", FlashFont, ActionObject)

FlashFont::FlashFont()
:	ActionObject("Object")
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
		m_shapes[i] = new FlashShape(0);
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
	m_coordinateType = coordinateType;
	
	for (AlignedVector< SwfKerningRecord >::const_iterator i = kerningRecords.begin(); i != kerningRecords.end(); ++i)
	{
		uint32_t codePair = (uint32_t(i->code1) << 16) | i->code2;
		m_kerningLookup[codePair] = i->adjustment;
	}

	for (uint32_t i = 0; i < codeTable.size(); ++i)
	{
		if (i >= 65535 || i >= shapeTable.size())
			break;

		m_indexLookup[codeTable[i]] = i;
	}

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

int16_t FlashFont::lookupKerning(uint16_t leftCode, uint16_t rightCode) const
{
	uint32_t codePair = (uint32_t(leftCode) << 16) | rightCode;
	SmallMap< uint32_t, int16_t >::const_iterator i = m_kerningLookup.find(codePair);
	return i != m_kerningLookup.end() ? i->second : 0;
}

uint16_t FlashFont::lookupIndex(uint16_t code) const
{
	SmallMap< uint16_t, uint16_t >::const_iterator i = m_indexLookup.find(code);
	return i != m_indexLookup.end() ? i->second : 0;
}

FlashFont::CoordinateType FlashFont::getCoordinateType() const
{
	return m_coordinateType;
}

void FlashFont::trace(const IVisitor& visitor) const
{
	for (RefArray< FlashShape >::const_iterator i = m_shapes.begin(); i != m_shapes.end(); ++i)
		visitor(*i);
	ActionObject::trace(visitor);
}

void FlashFont::dereference()
{
	m_shapes.clear();
	ActionObject::dereference();
}

	}
}
