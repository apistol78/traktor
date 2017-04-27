/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/ColorTransform.h"
#include "Flash/FlashLineStyle.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

FlashLineStyle::FlashLineStyle()
:	m_lineColor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_lineWidth(0)
{
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

void FlashLineStyle::transform(const ColorTransform& cxform)
{
	m_lineColor = m_lineColor * cxform.mul + cxform.add;
}

const Color4f& FlashLineStyle::getLineColor() const
{
	return m_lineColor;
}

uint16_t FlashLineStyle::getLineWidth() const
{
	return m_lineWidth;
}

void FlashLineStyle::serialize(ISerializer& s)
{
	s >> Member< Color4f >(L"lineColor", m_lineColor);
	s >> Member< uint16_t >(L"lineWidth", m_lineWidth);
}

	}
}
