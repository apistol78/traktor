/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TextureControl.h"
#include "Ui/Application.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::render
{
	namespace
	{

ui::Size operator * (const ui::Size& sz, float scale)
{
	return ui::Size(
		(int32_t)(sz.cx * scale),
		(int32_t)(sz.cy * scale)
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureControl", TextureControl, ui::Widget)

bool TextureControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &TextureControl::eventMouseDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &TextureControl::eventMouseUp);
	addEventHandler< ui::MouseMoveEvent >(this, &TextureControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &TextureControl::eventMouseWheel);
	addEventHandler< ui::PaintEvent >(this, &TextureControl::eventPaint);
	return true;
}

ui::Size TextureControl::getMinimumSize() const
{
	return m_image ? m_image->getSize() : ui::Size(0, 0);
}

ui::Size TextureControl::getPreferredSize(const ui::Size& hint) const
{
	return m_image ? m_image->getSize() : ui::Size(0, 0);
}

bool TextureControl::setImage(ui::IBitmap* image)
{
	m_image = image;
	m_offset = { 0, 0 };
	update();
	return true;
}

void TextureControl::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtLeft)
		return;

	m_moveOrigin = event->getPosition();
	m_moveOriginOffset = m_offset;

	setCapture();
}

void TextureControl::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void TextureControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	ui::Size offset = event->getPosition() - m_moveOrigin;
	m_offset = m_moveOriginOffset + offset;

	update();
}

void TextureControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	const int32_t rotation = event->getRotation();

	m_scale += rotation * 0.2f;
	m_scale = std::max(m_scale, 0.2f);

	update();
}

void TextureControl::eventPaint(ui::PaintEvent* event)
{
	const ui::StyleSheet* ss = getStyleSheet();
	ui::Canvas& canvas = event->getCanvas();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(getInnerRect());

	if (m_image)
	{
		ui::Size clientSize = getInnerRect().getSize();
		ui::Size imageSize = m_image->getSize();

		ui::Point center =
		{
			(clientSize.cx - imageSize.cx) / 2,
			(clientSize.cy - imageSize.cy) / 2
		};

		canvas.drawBitmap(
			center + m_offset,
			m_image->getSize() * m_scale,
			ui::Point(0, 0),
			m_image->getSize(),
			m_image,
			ui::BlendMode::Opaque,
			ui::Filter::Nearest
		);
	}

	event->consume();
}

}
