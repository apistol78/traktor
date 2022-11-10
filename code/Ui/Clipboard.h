/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class IClipboard;

/*! Clipboard
 * \ingroup UI
 */
class T_DLLCLASS Clipboard : public Object
{
	T_RTTI_CLASS;

public:
	Clipboard(IClipboard* clipboard);

	virtual ~Clipboard();

	void destroy();

	bool setObject(ISerializable* object);

	bool setText(const std::wstring& text);

	bool setImage(const drawing::Image* image);

	ClipboardContent getContentType() const;

	Ref< ISerializable > getObject() const;

	std::wstring getText() const;

	Ref< const drawing::Image > getImage() const;

private:
	IClipboard* m_clipboard;
};

	}
}

