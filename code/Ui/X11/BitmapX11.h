#pragma once

#include <cairo.h>
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapX11 : public ISystemBitmap
{
public:
	BitmapX11();

	virtual bool create(uint32_t width, uint32_t height) override final;

	virtual void destroy() override final;

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos) override final;

	virtual Ref< drawing::Image > getImage() const override final;

	virtual Size getSize() const override final;

	cairo_surface_t* getSurface() const { return m_surface; }

private:
	cairo_surface_t* m_surface;
};

	}
}

