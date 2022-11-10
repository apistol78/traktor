/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ClipboardWin32 : public IClipboard
{
public:
	ClipboardWin32();

	virtual void destroy();

	virtual bool setObject(ISerializable* object);

	virtual bool setText(const std::wstring& text);

	virtual bool setImage(const drawing::Image* image);

	virtual ClipboardContent getContentType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual std::wstring getText() const;

	virtual Ref< const drawing::Image > getImage() const;

private:
	uint32_t m_objectFormat;
};

	}
}

