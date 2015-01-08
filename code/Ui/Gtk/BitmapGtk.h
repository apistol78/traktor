#ifndef traktor_ui_BitmapGtk_H
#define traktor_ui_BitmapGtk_H

#include <gtkmm.h>
#include "Ui/Itf/IBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapGtk : public IBitmap
{
public:
	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const;

	const Cairo::RefPtr< Cairo::ImageSurface >& getSurface() const { return m_surface; }

private:
	Cairo::RefPtr< Cairo::ImageSurface > m_surface;
};

	}
}

#endif  // traktor_ui_BitmapGtk_H
