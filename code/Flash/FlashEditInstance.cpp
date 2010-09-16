#include "Core/Io/StringOutputStream.h"
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
:	FlashCharacterInstance(context, L"TextField", parent)
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

	Ref< const html::Element > element = document.getDocumentElement();
	for (element = element->getFirstElementChild(); element; element = element->getNextElementSibling())
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

	const FlashFont* font = movie->getFont(m_edit->getFontId());
	if (!font)
		return false;

	float fontScale =
		font->getCoordinateType() == FlashFont::CtTwips ? 
		1.0f / 1000.0f :
		1.0f / (20.0f * 1000.0f);
	float fontHeight = m_edit->getFontHeight();

	const float c_magicX = 32.0f * 20.0f;

	outWidth = 0.0f;
	outHeight = 0.0f;

	for (text_t::const_iterator i = m_text.begin(); i != m_text.end(); ++i)
	{
		const std::wstring& line = *i;
		
		float width = 0.0f;
		for (uint32_t j = 0; j < line.length(); ++j)
		{
			uint16_t glyphIndex = font->lookupIndex(line[j]);
			int16_t glyphAdvance = font->getAdvance(glyphIndex);
			if (j < line.length() - 1)
			{
				const SwfKerningRecord* kerningRecord = font->lookupKerningRecord(line[j], line[j + 1]);
				if (kerningRecord)
					glyphAdvance += kerningRecord->adjustment;
			}
			width += (glyphAdvance - c_magicX);
		}

		width *= fontScale * fontHeight;

		outWidth = max(outWidth, width);
		outHeight += fontHeight;
	}

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
