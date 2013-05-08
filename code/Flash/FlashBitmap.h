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

	FlashBitmap(drawing::Image* image);

	virtual ~FlashBitmap();

	/*! \brief Create bitmap from image.
	 *
	 * \param image Source image.
	 * \return True if successfully created.
	 */
	bool create(drawing::Image* image);

	uint32_t getWidth() const { return m_width; }

	uint32_t getHeight() const { return m_height; }

	uint32_t getMips() const { return m_mips; }

	const SwfColor* getBits() const { return m_bits.c_ptr(); }

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_mips;
	AutoArrayPtr< SwfColor > m_bits;
};

	}
}

#endif	// traktor_flash_FlashBitmap_H
