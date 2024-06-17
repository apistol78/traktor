/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::render
{

class IBitmap;
class TextureOutput;

/*! Texture preview control.
 * \ingroup Render
 */
class T_DLLCLASS TextureControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual ui::Size getMinimumSize() const override;

	virtual ui::Size getPreferredSize(const ui::Size& hint) const override;

	bool setImage(drawing::Image* image, const TextureOutput& output, uint32_t channels);

	bool getPixel(const ui::Point& position, Color4f& outColor) const;

private:
	Ref< drawing::Image > m_imageSource;
	Ref< drawing::Image > m_imageOutput;
	Ref< ui::IBitmap > m_bitmapSource;
	Ref< ui::IBitmap > m_bitmapOutput;
	ui::Size m_offset = { 0, 0 };
	ui::Point m_moveOrigin;
	ui::Size m_moveOriginOffset;
	float m_scale = 1.0f;

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

}
