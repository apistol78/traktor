#include "Ui/Clipboard.h"
#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Clipboard", Clipboard, Object)

Clipboard::Clipboard(IClipboard* clipboard)
:	m_clipboard(clipboard)
{
}

Clipboard::~Clipboard()
{
	T_ASSERT (m_clipboard);
	m_clipboard->destroy();
}

bool Clipboard::setObject(Serializable* object)
{
	T_ASSERT (m_clipboard);
	return m_clipboard->setObject(object);
}

bool Clipboard::setText(const std::wstring& text)
{
	T_ASSERT (m_clipboard);
	return m_clipboard->setText(text);
}

ClipboardContentType Clipboard::getContentType() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getContentType();
}

Serializable* Clipboard::getObject() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getObject();
}

std::wstring Clipboard::getText() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getText();
}

	}
}
