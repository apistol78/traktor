#ifndef traktor_ui_Bitmap_H
#define traktor_ui_Bitmap_H

#include "Core/Object.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class IBitmap;

/*! \brief Bitmap.
 * \ingroup UI
 *
 * The Bitmap class is a wrapper for system
 * dependent IBitmap instances.
 */
class T_DLLCLASS Bitmap : public Object
{
	T_RTTI_CLASS(Bitmap)

public:
	Bitmap();

	Bitmap(uint32_t width, uint32_t height);

	Bitmap(drawing::Image* image);

	Bitmap(drawing::Image* image, const ui::Rect& srcRect);

	virtual ~Bitmap();

	/*! \brief Create empty bitmap.
	 *
	 * \param width Width of bitmap.
	 * \param height Height of bitmap.
	 * \return True if successful.
	 */
	bool create(uint32_t width, uint32_t height);

	/*! \brief Create a copy of an image.
	 *
	 * \param image Source image.
	 * \return True if successful.
	 */
	bool create(drawing::Image* image);

	/*! \brief Create a sub-copy of an image.
	 *
	 * \param image Source image.
	 * \param srcRect Source rectangle from image.
	 * \return True if successful.
	 */
	bool create(drawing::Image* image, const ui::Rect& srcRect);

	/*! \brief Destroy bitmap. */
	virtual void destroy();

	/*! \brief Copy image into bitmap. */
	void copyImage(drawing::Image* image);

	/*! \brief Copy sub-rectangle from image into bitmap.
	 *
	 * \param image Source image.
	 * \param srcRect Source rectangle.
	 * \param destPos Destination position.
	 */
	void copySubImage(drawing::Image* image, const ui::Rect& srcRect, const ui::Point& destPos);

	/*! \brief Get image from bitmap. */
	drawing::Image* getImage() const;

	/*! \brief Get size of bitmap in pixels. */
	Size getSize() const;

	/*! \brief Get pixel. */
	Color getPixel(uint32_t x, uint32_t y) const;

	/*! \brief Set pixel. */
	void setPixel(uint32_t x, uint32_t y, const Color& color);

	/*! \brief Get bitmap implementation interface. */
	IBitmap* getIBitmap() const;

	/*! \brief Load bitmap from file. */
	static Bitmap* load(const std::wstring& fileName);

	/*! \brief Load bitmap from resource. */
	static Bitmap* load(const void* resource, uint32_t size, const std::wstring& extension);

private:
	IBitmap* m_bitmap;
};

	}
}

#endif	// traktor_ui_Bitmap_H
