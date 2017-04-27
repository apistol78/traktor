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

bool Image::create(Widget* parent, Bitmap* image, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_image = image;
	m_transparent = bool((style & WsTransparent) == WsTransparent);

	addEventHandler< PaintEvent >(this, &Image::eventPaint);

	return true;
}

Size Image::getMinimumSize() const
{
	return m_image ? m_image->getSize() : Size(0, 0);
}

Size Image::getPreferedSize() const
{
	return m_image ? m_image->getSize() : Size(0, 0);
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

		canvas.drawBitmap(
			Point(0, 0),
			Point(0, 0),
			m_image->getSize(),
			m_image,
			m_transparent ? BmAlpha : BmNone
		);
	}
}

	}
}
