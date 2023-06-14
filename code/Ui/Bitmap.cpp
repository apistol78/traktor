/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Bitmap", Bitmap, IBitmap)

Bitmap::Bitmap(uint32_t width, uint32_t height)
{
	create(width, height);
}

Bitmap::Bitmap(const drawing::Image* image)
{
	create(image);
}

Bitmap::Bitmap(const drawing::Image* image, const ui::Rect& srcRect)
{
	create(image, srcRect);
}

Bitmap::~Bitmap()
{
	destroy();
}

bool Bitmap::create(uint32_t width, uint32_t height)
{
	ISystemBitmap* bm;

	if (!(bm = Application::getInstance()->getWidgetFactory()->createBitmap()))
		return false;

	if (!bm->create(width, height))
	{
		safeDestroy(bm);
		return false;
	}

	m_bitmap = bm;
	return true;
}

bool Bitmap::create(const drawing::Image* image)
{
	if (!create(image->getWidth(), image->getHeight()))
		return false;

	m_bitmap->copySubImage(
		image,
		Rect(0, 0, image->getWidth(), image->getHeight()),
		Point(0, 0)
	);
	return true;
}

bool Bitmap::create(const drawing::Image* image, const Rect& srcRect)
{
	if (!create(image->getWidth(), image->getHeight()))
		return false;

	m_bitmap->copySubImage(
		image,
		srcRect,
		Point(0, 0)
	);
	return true;
}

void Bitmap::destroy()
{
	safeDestroy(m_bitmap);
}

void Bitmap::copyImage(const drawing::Image* image)
{
	if (m_bitmap)
		m_bitmap->copySubImage(
			image,
			Rect(0, 0, image->getWidth(), image->getHeight()),
			Point(0, 0)
		);
}

void Bitmap::copySubImage(const drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	if (m_bitmap)
		m_bitmap->copySubImage(image, srcRect, destPos);
}

Ref< drawing::Image > Bitmap::getImage(const Widget* reference) const
{
	return m_bitmap->getImage();
}

Size Bitmap::getSize(const Widget* reference) const
{
	return m_bitmap->getSize();
}

ISystemBitmap* Bitmap::getSystemBitmap(const Widget* reference) const
{
	return m_bitmap;
}

Ref< Bitmap > Bitmap::load(const std::wstring& fileName)
{
	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
		return nullptr;

	Ref< Bitmap > bitmap = new Bitmap();
	if (!bitmap->create(image))
		return nullptr;

	return bitmap;
}

Ref< Bitmap > Bitmap::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< drawing::Image > image = drawing::Image::load(resource, size, extension);
	if (!image)
		return nullptr;

	Ref< Bitmap > bitmap = new Bitmap();
	if (!bitmap->create(image))
		return nullptr;

	return bitmap;
}

}
