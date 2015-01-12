#ifndef traktor_ui_BitmapWx_H
#define traktor_ui_BitmapWx_H

#include <wx/wx.h>
#include "Ui/Itf/IBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapWx : public IBitmap
{
public:
	BitmapWx();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const;

	inline wxImage* getWxImage() { return m_image; }

private:
	wxImage* m_image;
};

	}
}

#endif	// traktor_ui_BitmapWx_H
