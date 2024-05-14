/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::ui
{

class Bitmap;

/*! Color static control.
 * \ingroup UI
 */
class T_DLLCLASS ColorControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style);

	void setColor(const Color4ub& color);

	Color4ub getColor() const;

	virtual Size getPreferredSize(const Size& hint) const override;

private:
	Color4ub m_color;
	Ref< drawing::Image > m_previewImage;
	Ref< Bitmap > m_previewBitmap;

	void updatePreview();

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

}
