#ifndef traktor_ui_BitmapX11_H
#define traktor_ui_BitmapX11_H

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

	virtual bool create(uint32_t width, uint32_t height) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos) T_OVERRIDE T_FINAL;

	virtual Ref< drawing::Image > getImage() const T_OVERRIDE T_FINAL;

	virtual Size getSize() const T_OVERRIDE T_FINAL;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color) T_OVERRIDE T_FINAL;

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const T_OVERRIDE T_FINAL;

	cairo_surface_t* getSurface() const { return m_surface; }

private:
	cairo_surface_t* m_surface;
};

	}
}

#endif  // traktor_ui_BitmapX11_H
