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

namespace traktor
{
	namespace render
	{

class IBitmap;

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

	bool setImage(ui::IBitmap* image);

private:
	Ref< ui::IBitmap > m_image;
	ui::Size m_offset = { 0, 0 };
	ui::Point m_moveOrigin;
	ui::Size m_moveOriginOffset;


	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}

