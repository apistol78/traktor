/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/ColorTransform.h"
#include "Spark/LineStyle.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor::spark
{

LineStyle::LineStyle()
:	m_lineColor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_lineWidth(0)
{
}

bool LineStyle::create(const SwfLineStyle* lineStyle)
{
	if (lineStyle)
	{
		m_lineColor = lineStyle->color;
		m_lineWidth = lineStyle->width;
	}
	return true;
}

bool LineStyle::create(const Color4f& lineColor, uint16_t lineWidth)
{
	m_lineColor = lineColor;
	m_lineWidth = lineWidth;
	return true;
}

void LineStyle::transform(const ColorTransform& cxform)
{
	m_lineColor = m_lineColor * cxform.mul + cxform.add;
}

const Color4f& LineStyle::getLineColor() const
{
	return m_lineColor;
}

uint16_t LineStyle::getLineWidth() const
{
	return m_lineWidth;
}

void LineStyle::serialize(ISerializer& s)
{
	s >> Member< Color4f >(L"lineColor", m_lineColor);
	s >> Member< uint16_t >(L"lineWidth", m_lineWidth);
}

}
