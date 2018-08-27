#include "Ui/Gtk/ClipboardGtk.h"

namespace traktor
{
	namespace ui
	{

void ClipboardGtk::destroy()
{
}

bool ClipboardGtk::setObject(ISerializable* object)
{
	return false;
}

bool ClipboardGtk::setText(const std::wstring& text)
{
	return false;
}

bool ClipboardGtk::setImage(const drawing::Image* image)
{
	return false;
}

ClipboardContentType ClipboardGtk::getContentType() const
{
	return CtObject;
}

Ref< ISerializable > ClipboardGtk::getObject() const
{
	return nullptr;
}

std::wstring ClipboardGtk::getText() const
{
	return L"";
}

Ref< const drawing::Image > ClipboardGtk::getImage() const
{
	return nullptr;
}

	}
}