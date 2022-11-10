/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

class ClipboardCocoa : public IClipboard
{
public:
	virtual ~ClipboardCocoa() {}

	virtual void destroy() override final;

	virtual bool setObject(ISerializable* object) override final;

	virtual bool setText(const std::wstring& text) override final;

	virtual bool setImage(const drawing::Image* image) override final;

	virtual ClipboardContent getContentType() const override final;

	virtual Ref< ISerializable > getObject() const override final;

	virtual std::wstring getText() const override final;

	virtual Ref< const drawing::Image > getImage() const override final;
};

	}
}

