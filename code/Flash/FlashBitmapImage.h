#ifndef traktor_flash_FlashBitmapImage_H
#define traktor_flash_FlashBitmapImage_H

#include "Core/Misc/AutoPtr.h"
#include "Flash/FlashBitmap.h"
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
class T_DLLCLASS FlashBitmapImage : public FlashBitmap
{
	T_RTTI_CLASS;

public:
	FlashBitmapImage();

	FlashBitmapImage(drawing::Image* image);

	virtual ~FlashBitmapImage();

	/*! \brief Create bitmap from image.
	 *
	 * \param image Source image.
	 * \return True if successfully created.
	 */
	bool create(drawing::Image* image);

	const SwfColor* getBits() const { return m_bits.c_ptr(); }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	AutoArrayPtr< SwfColor > m_bits;
};

	}
}

#endif	// traktor_flash_FlashBitmapImage_H
