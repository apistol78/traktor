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
	destroy();
}

void Clipboard::destroy()
{
	if (m_clipboard)
	{
		m_clipboard->destroy();
		m_clipboard = 0;
	}
}

bool Clipboard::setObject(ISerializable* object)
{
	T_ASSERT (m_clipboard);
	return m_clipboard->setObject(object);
}

bool Clipboard::setText(const std::wstring& text)
{
	T_ASSERT (m_clipboard);
	return m_clipboard->setText(text);
}

bool Clipboard::setImage(const drawing::Image* image)
{
	T_ASSERT (m_clipboard);
	return m_clipboard->setImage(image);
}

ClipboardContentType Clipboard::getContentType() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getContentType();
}

Ref< ISerializable > Clipboard::getObject() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getObject();
}

std::wstring Clipboard::getText() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getText();
}

Ref< const drawing::Image > Clipboard::getImage() const
{
	T_ASSERT (m_clipboard);
	return m_clipboard->getImage();
}

	}
}
