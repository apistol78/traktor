#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Flash/Bitmap.h"
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
class T_DLLCLASS BitmapImage : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapImage();

	BitmapImage(const drawing::Image* image);

	const void* getBits() const;

	const drawing::Image* getImage() const { return m_image; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< drawing::Image > m_image;
};

	}
}

