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

void FlashLineStyle::transform(const SwfCxTransform& cxform)
{
	m_lineColor.red =   uint8_t(((m_lineColor.red   / 255.0f) * cxform.red[0]   + cxform.red[1]  ) * 255.0f);
	m_lineColor.green = uint8_t(((m_lineColor.green / 255.0f) * cxform.green[0] + cxform.green[1]) * 255.0f);
	m_lineColor.blue =  uint8_t(((m_lineColor.blue  / 255.0f) * cxform.blue[0]  + cxform.blue[1] ) * 255.0f);
	m_lineColor.alpha = uint8_t(((m_lineColor.alpha / 255.0f) * cxform.alpha[0] + cxform.alpha[1]) * 255.0f);
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
