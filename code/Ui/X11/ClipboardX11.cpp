#include "Ui/X11/ClipboardX11.h"

namespace traktor
{
	namespace ui
	{

void ClipboardX11::destroy()
{
}

bool ClipboardX11::setObject(ISerializable* object)
{
	return false;
}

bool ClipboardX11::setText(const std::wstring& text)
{
	return false;
}

bool ClipboardX11::setImage(const drawing::Image* image)
{
	return false;
}

ClipboardContentType ClipboardX11::getContentType() const
{
	return CtObject;
}

Ref< ISerializable > ClipboardX11::getObject() const
{
	return nullptr;
}

std::wstring ClipboardX11::getText() const
{
	return L"";
}

Ref< const drawing::Image > ClipboardX11::getImage() const
{
	return nullptr;
}

	}
}