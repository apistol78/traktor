#pragma once

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
	virtual void destroy() override final;

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
	virtual Ref< drawing::Image > getImage() const override final;

	/*! \brief Get size of bitmap in pixels. */
	virtual Size getSize() const override final;

	/*! \brief Get system bitmap. */
	virtual ISystemBitmap* getSystemBitmap() const override final;

	/*! \brief Load bitmap from file. */
	static Ref< Bitmap > load(const std::wstring& fileName);

	/*! \brief Load bitmap from resource. */
	static Ref< Bitmap > load(const void* resource, uint32_t size, const std::wstring& extension);

private:
	ISystemBitmap* m_bitmap;
};

	}
}

