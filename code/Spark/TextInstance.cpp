#include "Spark/Font.h"
#include "Spark/Text.h"
#include "Spark/TextInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextInstance", TextInstance, CharacterInstance)

TextInstance::TextInstance(const CharacterInstance* parent)
:	CharacterInstance(parent)
,	m_height(0.0f)
{
}

void TextInstance::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& TextInstance::getText() const
{
	return m_text;
}

void TextInstance::setHeight(float height)
{
	m_height = height;
}

float TextInstance::getHeight() const
{
	return m_height;
}

void TextInstance::render(render::RenderContext* renderContext) const
{
	Matrix33 T = getFullTransform();

	for (uint32_t i = 0; i < m_text.length(); ++i)
	{
		wchar_t ch = m_text[i];

		if (m_font)
			m_font->render(renderContext, T, ch, m_height);

		T = translate(m_height, 0.0f) * T;
	}
}

	}
}
