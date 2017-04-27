/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Bitmap_H
#define traktor_ui_Bitmap_H

#include "Ui/IBitmap.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color4ub;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! \brief Bitmap.
 * \ingroup UI
 *
 * The Bitmap class is a wrapper for system
 * dependent IBitmap instances.
 */
class T_DLLCLASS Bitmap : public IBitmap
{
	T_RTTI_CLASS;

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
	virtual void destroy() T_OVERRIDE T_FINAL;

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
	virtual Ref< drawing::Image > getImage() const T_OVERRIDE T_FINAL;

	/*! \brief Get size of bitmap in pixels. */
	virtual Size getSize() const T_OVERRIDE T_FINAL;

	/*! \brief Get pixel. */
	Color4ub getPixel(uint32_t x, uint32_t y) const;

	/*! \brief Set pixel. */
	void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	/*! \brief Get system bitmap. */
	virtual ISystemBitmap* getSystemBitmap() const T_OVERRIDE T_FINAL;

	/*! \brief Load bitmap from file. */
	static Ref< Bitmap > load(const std::wstring& fileName);

	/*! \brief Load bitmap from resource. */
	static Ref< Bitmap > load(const void* resource, uint32_t size, const std::wstring& extension);

private:
	ISystemBitmap* m_bitmap;
	bool m_cached;
};

	}
}

#endif	// traktor_ui_Bitmap_H
