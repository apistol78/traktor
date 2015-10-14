#include "Spark/Editor/Shape/SvgStyle.h"

namespace traktor
{
	namespace spark
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgStyle", SvgStyle, Object)
	
SvgStyle::SvgStyle()
:	m_fillEnable(false)
,	m_fill(255, 255, 255, 255)
,	m_strokeEnable(false)
,	m_strokeWidth(1.0f)
,	m_stroke(0, 0, 0, 255)
,	m_opacity(1.0f)
{
}

void SvgStyle::setFillEnable(bool fillEnable)
{
	m_fillEnable = fillEnable;
}

bool SvgStyle::getFillEnable() const
{
	return m_fillEnable;
}

void SvgStyle::setFillGradient(const SvgGradient* fillGradient)
{
	m_fillGradient = fillGradient;
}

const SvgGradient* SvgStyle::getFillGradient() const
{
	return m_fillGradient;
}

void SvgStyle::setFill(const Color4ub& fill)
{
	m_fill = fill;
}

const Color4ub& SvgStyle::getFill() const
{
	return m_fill;
}

void SvgStyle::setStrokeEnable(bool strokeEnable)
{
	m_strokeEnable = strokeEnable;
}

bool SvgStyle::getStrokeEnable() const
{
	return m_strokeEnable;
}

void SvgStyle::setStrokeGradient(const SvgGradient* strokeGradient)
{
	m_strokeGradient = strokeGradient;
}

const SvgGradient* SvgStyle::getStrokeGradient() const
{
	return m_strokeGradient;
}

void SvgStyle::setStrokeWidth(float strokeWidth)
{
	m_strokeWidth = strokeWidth;
}

float SvgStyle::getStrokeWidth() const
{
	return m_strokeWidth;
}

void SvgStyle::setStroke(const Color4ub& stroke)
{
	m_stroke = stroke;
}

const Color4ub& SvgStyle::getStroke() const
{
	return m_stroke;
}

void SvgStyle::setOpacity(float opacity)
{
	m_opacity = opacity;
}

float SvgStyle::getOpacity() const
{
	return m_opacity;
}

bool SvgStyle::operator == (const SvgStyle& other) const
{
	if (m_fillEnable != other.m_fillEnable)
		return false;

	if (m_fillEnable)
	{
		if (m_fillGradient != other.m_fillGradient)
			return false;
		if (m_fill != other.m_fill)
			return false;
	}

	if (m_strokeEnable != other.m_strokeEnable)
		return false;

	if (m_strokeEnable)
	{
		if (m_strokeGradient != other.m_strokeGradient)
			return false;
		if (m_strokeWidth != other.m_strokeWidth)
			return false;
		if (m_stroke != other.m_stroke)
			return false;
	}

	if (m_opacity != other.m_opacity)
		return false;

	return true;
}

	}
}
