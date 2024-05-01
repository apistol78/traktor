/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cairo.h>
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor::ui
{

class BitmapX11 : public ISystemBitmap
{
public:
	virtual bool create(uint32_t width, uint32_t height) override final;

	virtual void destroy() override final;

	virtual void copySubImage(const drawing::Image* image, const Rect& srcRect, const Point& destPos) override final;

	virtual Ref< drawing::Image > getImage() const override final;

	virtual Size getSize() const override final;

	cairo_surface_t* getSurface() const { return m_surface; }

private:
	cairo_surface_t* m_surface = nullptr;
};

}
