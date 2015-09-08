#include "Spark/Editor/Shape/Style.h"

namespace traktor
{
	namespace spark
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Style", Style, Object)
	
Style::Style()
:	m_fillEnable(false)
,	m_fill(0xffffff)
,	m_strokeEnable(false)
,	m_strokeWidth(1.0f)
,	m_stroke(0x000000)
{
}

void Style::setFillEnable(bool fillEnable)
{
	m_fillEnable = fillEnable;
}

bool Style::getFillEnable() const
{
	return m_fillEnable;
}

void Style::setFillGradient(Gradient* fillGradient)
{
	m_fillGradient = fillGradient;
}

Gradient* Style::getFillGradient() const
{
	return m_fillGradient;
}

void Style::setFill(color_t fill)
{
	m_fill = fill;
}

Style::color_t Style::getFill() const
{
	return m_fill;
}

void Style::setStrokeEnable(bool strokeEnable)
{
	m_strokeEnable = strokeEnable;
}

bool Style::getStrokeEnable() const
{
	return m_strokeEnable;
}

void Style::setStrokeGradient(Gradient* strokeGradient)
{
	m_strokeGradient = strokeGradient;
}

Gradient* Style::getStrokeGradient() const
{
	return m_strokeGradient;
}

void Style::setStrokeWidth(float strokeWidth)
{
	m_strokeWidth = strokeWidth;
}

float Style::getStrokeWidth() const
{
	return m_strokeWidth;
}

void Style::setStroke(color_t stroke)
{
	m_stroke = stroke;
}

Style::color_t Style::getStroke() const
{
	return m_stroke;
}

	}
}


