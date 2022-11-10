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

#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapCocoa : public ISystemBitmap
{
public:
	virtual bool create(uint32_t width, uint32_t height) override final;

	virtual void destroy() override final;

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos) override final;

	virtual Ref< drawing::Image > getImage() const override final;

	virtual Size getSize() const override final;

	inline NSImage* getNSImage() const { return m_image; }

	inline NSImage* getNSImagePreAlpha() const { return m_imagePreAlpha; }

private:
	NSImage* m_image = nullptr;
	NSBitmapImageRep* m_imageRep = nullptr;
	NSImage* m_imagePreAlpha = nullptr;
	NSBitmapImageRep* m_imageRepPreAlpha = nullptr;
};

	}
}

