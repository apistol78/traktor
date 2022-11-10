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
#include "Core/Config.h"
#include "Core/Ref.h"
#include "Ui/Enums.h"

namespace traktor
{

class ISerializable;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! Clipboard interface.
 * \ingroup UI
 */
class IClipboard
{
public:
	virtual void destroy() = 0;

	virtual bool setObject(ISerializable* object) = 0;

	virtual bool setText(const std::wstring& text) = 0;

	virtual bool setImage(const drawing::Image* image) = 0;

	virtual ClipboardContent getContentType() const = 0;

	virtual Ref< ISerializable > getObject() const = 0;

	virtual std::wstring getText() const = 0;

	virtual Ref< const drawing::Image > getImage() const = 0;
};

	}
}

