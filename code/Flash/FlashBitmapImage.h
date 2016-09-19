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

	FlashBitmapImage(const drawing::Image* image);

	const void* getBits() const;

	const drawing::Image* getImage() const { return m_image; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< drawing::Image > m_image;
};

	}
}

#endif	// traktor_flash_FlashBitmapImage_H
