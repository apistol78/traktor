#ifndef traktor_flash_FlashBitmap_H
#define traktor_flash_FlashBitmap_H

#include "Flash/Action/ActionObject.h"
#include "Flash/SwfTypes.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace flash
	{

/*! \brief Flash bitmap container.
 * \ingroup Flash
 */
class T_DLLCLASS FlashBitmap : public ActionObject
{
	T_RTTI_CLASS(FlashBitmap)

public:
	FlashBitmap();

	virtual ~FlashBitmap();

	/*! \brief Create bitmap from image.
	 *
	 * \param image Source image.
	 * \return True if successfully created.
	 */
	bool create(drawing::Image* image);

	/*! \brief Create empty bitmap.
	 *
	 * \param width Bitmap width.
	 * \param height Bitmap height.
	 * \return True if successfully created.
	 */
	bool create(uint16_t width, uint16_t height);

	uint16_t getOriginalWidth() const;

	uint16_t getOriginalHeight() const;
	
	uint16_t getBitsWidth() const;
	
	uint16_t getBitsHeight() const;

	const SwfColor* getBits() const;

	void setPixel(uint16_t x, uint16_t y, const SwfColor& color);

private:
	uint16_t m_originalWidth;
	uint16_t m_originalHeight;
	uint16_t m_bitsWidth;
	uint16_t m_bitsHeight;
	AutoArrayPtr< SwfColor > m_bits;
};

	}
}

#endif	// traktor_flash_FlashBitmap_H
