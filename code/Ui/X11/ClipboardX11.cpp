/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Ui/X11/ClipboardX11.h"

namespace traktor::ui
{

void ClipboardX11::destroy()
{
}

bool ClipboardX11::setObject(ISerializable* object)
{
	m_type = ClipboardContent::Object;
	m_object = object;
	return true;
}

bool ClipboardX11::setText(const std::wstring& text)
{
	m_type = ClipboardContent::Text;
	m_text = text;
	return true;
}

bool ClipboardX11::setImage(const drawing::Image* image)
{
	m_type = ClipboardContent::Image;
	m_image = image;
	return true;
}

ClipboardContent ClipboardX11::getContentType() const
{
	return m_type;
}

Ref< ISerializable > ClipboardX11::getObject() const
{
	return DeepClone(m_object).create();
}

std::wstring ClipboardX11::getText() const
{
	return m_text;
}

Ref< const drawing::Image > ClipboardX11::getImage() const
{
	return m_image;
}

}
