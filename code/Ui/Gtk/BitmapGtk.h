#ifndef traktor_ui_BitmapGtk_H
#define traktor_ui_BitmapGtk_H

#include <gtk/gtk.h>
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapGtk : public ISystemBitmap
{
public:
	BitmapGtk();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const;

	cairo_surface_t* getSurface() const { return m_surface; }

private:
	cairo_surface_t* m_surface;
};

	}
}

#endif  // traktor_ui_BitmapGtk_H
