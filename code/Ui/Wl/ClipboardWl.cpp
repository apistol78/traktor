/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Ui/Wl/ClipboardWl.h"

namespace traktor::ui
{

void ClipboardWl::destroy()
{
}

bool ClipboardWl::setObject(ISerializable* object)
{
	m_type = ClipboardContent::Object;
	m_object = object;
	return true;
}

bool ClipboardWl::setText(const std::wstring& text)
{
	m_type = ClipboardContent::Text;
	m_text = text;
	return true;
}

bool ClipboardWl::setImage(const drawing::Image* image)
{
	m_type = ClipboardContent::Image;
	m_image = image;
	return true;
}

ClipboardContent ClipboardWl::getContentType() const
{
	return m_type;
}

Ref< ISerializable > ClipboardWl::getObject() const
{
	return DeepClone(m_object).create();
}

std::wstring ClipboardWl::getText() const
{
	return m_text;
}

Ref< const drawing::Image > ClipboardWl::getImage() const
{
	return m_image;
}

}
