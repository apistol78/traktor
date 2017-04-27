/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_BitmapCocoa_H
#define traktor_ui_BitmapCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

class BitmapCocoa : public ISystemBitmap
{
public:
	BitmapCocoa();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const;
	
	inline NSImage* getNSImage() const { return m_image; }
	
	inline NSImage* getNSImagePreAlpha() const { return m_imagePreAlpha; }
	
private:
	NSImage* m_image;
	NSBitmapImageRep* m_imageRep;
	NSImage* m_imagePreAlpha;
	NSBitmapImageRep* m_imageRepPreAlpha;
};

	}
}

#endif	// traktor_ui_BitmapCocoa_H
