#ifndef traktor_ui_BitmapCocoa_H
#define traktor_ui_BitmapCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapCocoa : public IBitmap
{
public:
	BitmapCocoa();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual drawing::Image* getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color& color);

	virtual Color getPixel(uint32_t x, uint32_t y) const;
	
	inline NSImage* getNSImage() const { return m_image; }
	
private:
	NSImage* m_image;
	NSBitmapImageRep* m_imageRep;
};

	}
}

#endif	// traktor_ui_BitmapCocoa_H
