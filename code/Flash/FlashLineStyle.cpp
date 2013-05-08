#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashLineStyle.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

FlashLineStyle::FlashLineStyle()
:	m_lineWidth(0)
{
	m_lineColor.red =
	m_lineColor.green =
	m_lineColor.blue = 0;
	m_lineColor.alpha = 255;
}

bool FlashLineStyle::create(const SwfLineStyle* lineStyle)
{
	if (lineStyle)
	{
		m_lineColor = lineStyle->color;
		m_lineWidth = lineStyle->width;
	}
	return true;
}

const SwfColor& FlashLineStyle::getLineColor() const
{
	return m_lineColor;
}

uint16_t FlashLineStyle::getLineWidth() const
{
	return m_lineWidth;
}

void FlashLineStyle::serialize(ISerializer& s)
{
	s >> MemberSwfColor(L"lineColor", m_lineColor);
	s >> Member< uint16_t >(L"lineWidth", m_lineWidth);
}

	}
}
