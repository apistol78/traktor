/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Clipboard.h"
#include "Ui/Itf/IClipboard.h"

namespace traktor::ui
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
	safeDestroy(m_clipboard);
}

bool Clipboard::setObject(ISerializable* object)
{
	T_ASSERT(m_clipboard);
	return m_clipboard->setObject(object);
}

bool Clipboard::setText(const std::wstring& text)
{
	T_ASSERT(m_clipboard);
	return m_clipboard->setText(text);
}

bool Clipboard::setImage(const drawing::Image* image)
{
	T_ASSERT(m_clipboard);
	return m_clipboard->setImage(image);
}

ClipboardContent Clipboard::getContentType() const
{
	T_ASSERT(m_clipboard);
	return m_clipboard->getContentType();
}

Ref< ISerializable > Clipboard::getObject() const
{
	T_ASSERT(m_clipboard);
	return m_clipboard->getObject();
}

std::wstring Clipboard::getText() const
{
	T_ASSERT(m_clipboard);
	return m_clipboard->getText();
}

Ref< const drawing::Image > Clipboard::getImage() const
{
	T_ASSERT(m_clipboard);
	return m_clipboard->getImage();
}

}
