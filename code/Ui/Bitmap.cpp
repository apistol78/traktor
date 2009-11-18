#include "Ui/Bitmap.h"
#include "Ui/Application.h"
#include "Ui/Itf/IBitmap.h"
#include "Drawing/Image.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Bitmap", Bitmap, Object)

Bitmap::Bitmap()
:	m_bitmap(0)
{
}

Bitmap::Bitmap(uint32_t width, uint32_t height)
:	m_bitmap(0)
{
	create(width, height);
	T_ASSERT (m_bitmap);
}

Bitmap::Bitmap(drawing::Image* image)
:	m_bitmap(0)
{
	create(image);
	T_ASSERT (m_bitmap);
}

Bitmap::Bitmap(drawing::Image* image, const ui::Rect& srcRect)
:	m_bitmap(0)
{
	create(image, srcRect);
	T_ASSERT (m_bitmap);
}

Bitmap::~Bitmap()
{
	destroy();
}

bool Bitmap::create(uint32_t width, uint32_t height)
{
	if (!(m_bitmap = Application::getInstance().getWidgetFactory()->createBitmap()))
	{
		log::error << L"Failed to create native widget peer (Bitmap)" << Endl;
		return false;
	}

	if (!m_bitmap->create(width, height))
	{
		m_bitmap->destroy();
		m_bitmap = 0;
		return false;
	}

	return true;
}

bool Bitmap::create(drawing::Image* image)
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

bool Bitmap::create(drawing::Image* image, const Rect& srcRect)
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
	if (m_bitmap)
	{
		m_bitmap->destroy();
		m_bitmap = 0;
	}
}

void Bitmap::copyImage(drawing::Image* image)
{
	if (m_bitmap)
		m_bitmap->copySubImage(
			image,
			Rect(0, 0, image->getWidth(), image->getHeight()),
			Point(0, 0)
		);
}

void Bitmap::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	if (m_bitmap)
		m_bitmap->copySubImage(image, srcRect, destPos);
}

Ref< drawing::Image > Bitmap::getImage() const
{
	if (!m_bitmap)
		return 0;

	return m_bitmap->getImage();
}

Size Bitmap::getSize() const
{
	if (!m_bitmap)
		return Size(0, 0);

	return m_bitmap->getSize();
}

Color Bitmap::getPixel(uint32_t x, uint32_t y) const
{
	if (!m_bitmap)
		return Color(0, 0, 0);

	Size sz = m_bitmap->getSize();
	if (x >= sz.cx || y >= sz.cy)
		return Color(0, 0, 0);

	return m_bitmap->getPixel(x, y);
}

void Bitmap::setPixel(uint32_t x, uint32_t y, const Color& color)
{
	if (!m_bitmap)
		return;

	Size sz = m_bitmap->getSize();
	if (x >= sz.cx || y >= sz.cy)
		return;

	m_bitmap->setPixel(x, y, color);
}

IBitmap* Bitmap::getIBitmap() const
{
	return m_bitmap;
}

Ref< Bitmap > Bitmap::load(const std::wstring& fileName)
{
	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
		return 0;

	Ref< Bitmap > bitmap = new Bitmap();
	if (!bitmap->create(image))
		return 0;

	return bitmap;
}

Ref< Bitmap > Bitmap::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< drawing::Image > image = drawing::Image::load(resource, size, extension);
	if (!image)
		return 0;

	Ref< Bitmap > bitmap = new Bitmap();
	if (!bitmap->create(image))
		return 0;

	return bitmap;
}

	}
}
