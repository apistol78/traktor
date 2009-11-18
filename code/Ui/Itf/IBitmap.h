#ifndef traktor_ui_IBitmap_H
#define traktor_ui_IBitmap_H

#include "Core/Config.h"
#include "Core/Ref.h"
#include "Ui/Rect.h"

namespace traktor
{

class Color;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! \brief Bitmap interface.
 * \ingroup UI
 */
class IBitmap
{
public:
	virtual bool create(uint32_t width, uint32_t height) = 0;

	virtual void destroy() = 0;

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos) = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual Size getSize() const = 0;

	virtual void setPixel(uint32_t x, uint32_t y, const Color& color) = 0;

	virtual Color getPixel(uint32_t x, uint32_t y) const = 0;
};

	}
}

#endif	// traktor_ui_IBitmap_H
