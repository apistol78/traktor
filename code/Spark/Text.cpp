#include "Spark/Font.h"
#include "Spark/Text.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Text", Text, Character)

Text::Text(const Character* parent, const resource::Proxy< Font >& font)
:	Character(parent)
,	m_font(font)
,	m_height(0.0f)
,	m_horizontalAlign(AnLeft)
,	m_verticalAlign(AnTop)
{
}

void Text::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Text::getText() const
{
	return m_text;
}

void Text::setHeight(float height)
{
	m_height = height;
}

float Text::getHeight() const
{
	return m_height;
}

void Text::setBounds(const Aabb2& bounds)
{
	m_bounds = bounds;
}

void Text::setHorizontalAlign(Align horizontalAlign)
{
	m_horizontalAlign = horizontalAlign;
}

Align Text::getHorizontalAlign() const
{
	return m_horizontalAlign;
}

void Text::setVerticalAlign(Align verticalAlign)
{
	m_verticalAlign = verticalAlign;
}

Align Text::getVerticalAlign() const
{
	return m_verticalAlign;
}

void Text::setAlpha(float alpha)
{
	m_colorTransform.alpha[0] = alpha;
}

float Text::getAlpha() const
{
	return m_colorTransform.alpha[0];
}

Aabb2 Text::getBounds() const
{
	return m_bounds;
}

void Text::update()
{
}

void Text::render(render::RenderContext* renderContext) const
{
	if (!m_font || !m_visible)
		return;

	// Measure text extent.
	float extentX = 0.0f;
	float extentY = m_height;
	for (uint32_t i = 0; i < m_text.length(); ++i)
	{
		wchar_t ch = m_text[i];
		extentX += m_font->advance(ch) * m_height;
	}

	float boundsX = m_bounds.mx.x - m_bounds.mn.x;
	float boundsY = m_bounds.mx.y - m_bounds.mn.y;

	float offsetX = 0.0f;
	float offsetY = 0.0f;

	switch (m_horizontalAlign)
	{
	default:
	case AnLeft:
		break;

	case AnCenter:
		offsetX = (boundsX - extentX) / 2.0f;
		break;

	case AnRight:
		offsetX = boundsX - extentX;
		break;
	}

	switch (m_verticalAlign)
	{
	default:
	case AnTop:
		break;

	case AnCenter:
		offsetY = (boundsY - extentY) / 2.0f;
		break;

	case AnBottom:
		offsetY = boundsY - extentY;
		break;
	}

	Matrix33 T = getFullTransform();
	T = T * translate(m_bounds.mn.x + offsetX, m_bounds.mn.y + offsetY);

	// Render glyphs.
	for (uint32_t i = 0; i < m_text.length(); ++i)
	{
		wchar_t ch = m_text[i];
		m_font->render(renderContext, T, ch, m_height, m_colorTransform);
		T = T * translate(m_font->advance(ch) * m_height, 0.0f);
	}
}

	}
}
