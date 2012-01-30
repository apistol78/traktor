#ifndef traktor_flash_FlashBitmap_H
#define traktor_flash_FlashBitmap_H

#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS FlashBitmap : public ISerializable
{
	T_RTTI_CLASS;

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

	uint16_t getWidth() const { return m_width; }

	uint16_t getHeight() const { return m_height; }

	const SwfColor* getBits() const { return m_bits.c_ptr(); }

	void setPixel(uint16_t x, uint16_t y, const SwfColor& color)
	{
		if (x < m_width && y < m_height)
			m_bits[x + y * m_width] = color;
	}

	virtual bool serialize(ISerializer& s);

private:
	uint16_t m_width;
	uint16_t m_height;
	AutoArrayPtr< SwfColor > m_bits;
};

	}
}

#endif	// traktor_flash_FlashBitmap_H
