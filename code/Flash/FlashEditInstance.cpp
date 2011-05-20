#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Thread/Acquire.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMovie.h"
#include "Flash/Action/ActionContext.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

void concateHtmlText(const html::Node* node, StringOutputStream& ss)
{
	if (const html::Text* text = dynamic_type_cast< const html::Text* >(node))
		ss << text->getValue();

	for (html::Node* child = node->getFirstChild(); child; child = child->getNextSibling())
		concateHtmlText(child, ss);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditInstance", FlashEditInstance, FlashCharacterInstance)

FlashEditInstance::FlashEditInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html)
:	FlashCharacterInstance(context, "TextField", parent)
,	m_edit(edit)
{
	if (m_edit->renderHtml())
		parseHtml(html);
	else
		parseText(html);
}

const FlashEdit* FlashEditInstance::getEdit() const
{
	return m_edit;
}

bool FlashEditInstance::parseText(const std::wstring& text)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_text.clear();
	m_text.push_back(text);
	return true;
}

bool FlashEditInstance::parseHtml(const std::wstring& html)
{
	html::Document document(false);

	if (!document.loadFromText(html))
		return false;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_text.clear();

	const html::Element* element = document.getDocumentElement();
	for (element = element->getFirstElementChild().ptr(); element; element = element->getNextElementSibling())
	{
		StringOutputStream ss;
		concateHtmlText(element, ss);
		if (!ss.empty())
			m_text.push_back(ss.str());
	}

	return true;
}

FlashEditInstance::text_t FlashEditInstance::getText() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_text;
}

bool FlashEditInstance::getTextExtents(float& outWidth, float& outHeight) const
{
	const FlashMovie* movie = getContext()->getMovie();
	T_ASSERT (movie);

	outWidth = 0;
	outHeight = 0;

	const FlashFont* font = movie->getFont(m_edit->getFontId());
	if (!font)
		return false;

	float fontScale =
		font->getCoordinateType() == FlashFont::CtTwips ? 
		1.0f / 1000.0f :
		1.0f / (20.0f * 1000.0f);
	float fontHeight = m_edit->getFontHeight();

	const float c_magicX = 32.0f * 20.0f;

	float offsetY = 0.0f;

	// Get space width.
	uint16_t spaceGlyphIndex = font->lookupIndex(L' ');
	int16_t spaceWidth = font->getAdvance(spaceGlyphIndex);

	// Render text lines.
	for (FlashEditInstance::text_t::const_iterator i = m_text.begin(); i != m_text.end(); ++i)
	{
		std::vector< std::wstring > words;
		Split< std::wstring >::any(*i, L" \t", words);

		// Calculate width of each word.
		std::vector< float > widths(words.size());
		for (uint32_t j = 0; j < words.size(); ++j)
		{
			const std::wstring& word = words[j];
			uint32_t wordLength = word.length();

			float wordWidth = 0.0f;
			for (uint32_t k = 0; k < wordLength; ++k)
			{
				uint16_t glyphIndex = font->lookupIndex(word[k]);
				int16_t glyphAdvance = font->getAdvance(glyphIndex);
				if (k < wordLength - 1)
					glyphAdvance += font->lookupKerning(word[k], word[k + 1]);
				wordWidth += (glyphAdvance - c_magicX);
			}

			widths[j] = wordWidth * fontScale * fontHeight;
		}

		// Pack as many words as fits in bounds (only if word wrap enabled).
		uint32_t wordOffsetStart = 0;
		uint32_t wordOffsetEnd = 0;
		const FlashEdit* edit = getEdit();
		const SwfRect& bounds = edit->getTextBounds();
		const SwfColor& color = edit->getTextColor();
		bool wordWrap = edit->wordWrap();

		while (wordOffsetStart < words.size())
		{
			float lineWidth = 0.0f;
			while (wordOffsetEnd < words.size())
			{
				float wordWidth = widths[wordOffsetEnd];

				if (wordOffsetStart >= wordOffsetEnd)
					lineWidth = wordWidth;
				else
				{
					wordWidth += spaceWidth * fontScale * fontHeight;
					if (wordWrap && lineWidth + wordWidth >= bounds.max.x - bounds.min.x)
						break;
					lineWidth += wordWidth;
				}

				wordOffsetEnd++;
			}

			// Calculate line horizontal offset.
			float offsetX = 0.0f;
			if (edit->getAlign() == FlashEdit::AnCenter)
				offsetX = (bounds.max.x - bounds.min.x - lineWidth) / 2.0f;
			else if (edit->getAlign() == FlashEdit::AnRight)
				offsetX = bounds.max.x - bounds.min.x - lineWidth;

			// Render each word.
			while (wordOffsetStart < wordOffsetEnd)
			{
				const std::wstring& word = words[wordOffsetStart++];
				uint32_t wordLength = word.length();

				for (uint32_t i = 0; i < wordLength; ++i)
				{
					wchar_t ch = word[i];
					uint16_t glyphIndex = font->lookupIndex(ch);

					int16_t glyphAdvance = font->getAdvance(glyphIndex);
					if (i < wordLength - 1)
						glyphAdvance += font->lookupKerning(word[i], word[i + 1]);

					offsetX += (glyphAdvance - c_magicX) * fontScale * fontHeight;
				}
				offsetX += spaceWidth * fontScale * fontHeight;
			}
			outWidth = max(outWidth, offsetX);
			offsetY += fontHeight;
		}
	}
	outHeight = offsetY;
	outWidth /= 20.0f;
	outHeight /= 20.0f;
	return true;
}

SwfRect FlashEditInstance::getBounds() const
{
	SwfRect textBounds = m_edit->getTextBounds();
	textBounds.min = getTransform() * textBounds.min;
	textBounds.max = getTransform() * textBounds.max;
	return textBounds;
}

	}
}
