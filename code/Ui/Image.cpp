/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Image.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Image", Image, Widget)

Image::Image()
:	m_transparent(false)
,	m_scale(false)
,	m_keepAspect(false)
{
}

bool Image::create(Widget* parent, Bitmap* image, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_image = image;
	m_transparent = bool((style & WsTransparent) == WsTransparent);
	m_scale = bool((style & (WsScale | WsScaleKeepAspect)) != 0);
	m_keepAspect = bool((style & WsScaleKeepAspect) == WsScaleKeepAspect);

	addEventHandler< PaintEvent >(this, &Image::eventPaint);

	return true;
}

Size Image::getMinimumSize() const
{
	if (!m_image)
		return Size(0, 0);

	if (!m_scale)
		return m_image->getSize();
	else
		return Size(0, 0);
}

Size Image::getPreferedSize() const
{
	if (!m_image)
		return Size(0, 0);

	if (m_scale && m_keepAspect)
	{
		Size csz = getInnerRect().getSize();
		Size isz = m_image->getSize();

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
		return m_image->getSize();
}

Size Image::getMaximumSize() const
{
	return m_image ? m_image->getSize() : Size(0, 0);
}

bool Image::setImage(Bitmap* image, bool transparent)
{
	m_image = image;
	m_transparent = transparent;
	
	setRect(Rect(getRect().getTopLeft(), getPreferedSize()));
	update();

	return true; 
}

Ref< Bitmap > Image::getImage() const
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
	if (m_image)
	{
		Canvas& canvas = event->getCanvas();

		if (m_transparent)
		{
			canvas.fillRect(
				getInnerRect()
			);
		}

		if (!m_scale)
			canvas.drawBitmap(
				Point(0, 0),
				Point(0, 0),
				m_image->getSize(),
				m_image,
				m_transparent ? BmAlpha : BmNone
			);
		else
			canvas.drawBitmap(
				Point(0, 0),
				getInnerRect().getSize(),
				Point(0, 0),
				m_image->getSize(),
				m_image,
				m_transparent ? BmAlpha : BmNone
			);
	}
}

	}
}
