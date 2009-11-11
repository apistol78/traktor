#include "Flash/FlashEditInstance.h"
#include "Flash/FlashEdit.h"
#include "Flash/Action/Classes/AsTextField.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Core/Io/StringOutputStream.h"

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
:	FlashCharacterInstance(context, AsTextField::getInstance(), parent)
,	m_edit(edit)
{
	if (m_edit->renderHtml())
		parseHtml(html);
	else
		parseText(html);
}

Ref< const FlashEdit > FlashEditInstance::getEdit() const
{
	return m_edit;
}

bool FlashEditInstance::parseText(const std::wstring& text)
{
	m_text.clear();
	m_text.push_back(text);
	return true;
}

bool FlashEditInstance::parseHtml(const std::wstring& html)
{
	html::Document document(false);

	if (!document.loadFromText(html))
		return false;

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

const FlashEditInstance::text_t& FlashEditInstance::getText() const
{
	return m_text;
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
