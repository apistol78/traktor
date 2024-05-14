/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/IBitmap.h"
#include "Ui/Image.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Image", Image, Widget)

bool Image::create(Widget* parent, IBitmap* image, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_image = image;
	m_transparent = bool((style & WsTransparent) == WsTransparent);
	m_scale = bool((style & (WsScale | WsScaleKeepAspect)) != 0);
	m_keepAspect = bool((style & WsScaleKeepAspect) == WsScaleKeepAspect);
	m_nearest = bool((style & WsNearestFilter) == WsNearestFilter);

	addEventHandler< PaintEvent >(this, &Image::eventPaint);
	return true;
}

Size Image::getMinimumSize() const
{
	if (!m_image)
		return Size(0, 0);

	if (!m_scale)
		return m_image->getSize(this);
	else
		return Size(0, 0);
}

Size Image::getPreferredSize(const Size& hint) const
{
	if (!m_image)
		return Size(0, 0);

	if (m_scale && m_keepAspect)
	{
		Size csz = getInnerRect().getSize();
		Size isz = m_image->getSize(this);

		if (csz.cx < isz.cx)
		{
			isz.cy = (csz.cx * isz.cy) / isz.cx;
			isz.cx = csz.cx;
		}

		if (csz.cy < isz.cy)
		{
			isz.cx = (csz.cy * isz.cx) / isz.cy;
			isz.cy = csz.cy;
		}

		return isz;
	}
	else
		return m_image->getSize(this);
}

Size Image::getMaximumSize() const
{
	return m_image ? m_image->getSize(this) : Size(0, 0);
}

bool Image::setImage(IBitmap* image, bool transparent)
{
	m_image = image;
	m_transparent = transparent;

	if (!m_scale)
	{
		const Rect rc = getRect();
		setRect(Rect(rc.getTopLeft(), getPreferredSize(rc.getSize())));
	}

	update();
	return true;
}

IBitmap* Image::getImage() const
{
	return m_image;
}

bool Image::isTransparent() const
{
	return m_transparent;
}

bool Image::scaling() const
{
	return m_scale;
}

void Image::eventPaint(PaintEvent* event)
{
	if (!m_image)
		return;

	const StyleSheet* ss = getStyleSheet();
	Canvas& canvas = event->getCanvas();

	const Size imageSize = m_image->getSize(this);
	const Size clientSize = getInnerRect().getSize();
	Size drawSize = clientSize;

	if (m_transparent || m_keepAspect)
	{
		canvas.setBackground(ss->getColor(this, L"background-color"));
		canvas.fillRect(getInnerRect());
	}

	if (!m_scale)
	{
		canvas.drawBitmap(
			Point(0, 0),
			Point(0, 0),
			imageSize,
			m_image,
			m_transparent ? BlendMode::Alpha : BlendMode::Opaque,
			m_nearest ? Filter::Nearest : Filter::Linear
		);
	}
	else
	{
		Point offset(0, 0);

		if (m_keepAspect)
		{
			const float imageAspect = (float)imageSize.cx / imageSize.cy;
			const float drawAspect = (float)drawSize.cx / drawSize.cy;
			if (drawAspect > imageAspect)
				drawSize.cx = (int32_t)(drawSize.cy * imageAspect);
			else
				drawSize.cy = (int32_t)(drawSize.cx / imageAspect);

			offset.x = (clientSize.cx - drawSize.cx) / 2;
			offset.y = (clientSize.cy - drawSize.cy) / 2;
		}

		canvas.drawBitmap(
			offset,
			drawSize,
			Point(0, 0),
			imageSize,
			m_image,
			m_transparent ? BlendMode::Alpha : BlendMode::Opaque,
			m_nearest ? Filter::Nearest : Filter::Linear
		);
	}
	
	event->consume();
}

}
