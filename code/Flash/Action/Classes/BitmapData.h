#ifndef traktor_flash_BitmapData_H
#define traktor_flash_BitmapData_H

#include "Flash/Action/ActionObjectRelay.h"

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

/*! \brief ActionScript boolean wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS BitmapData : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	BitmapData(int32_t width, int32_t height, bool transparent, uint32_t fillColor);

	BitmapData(drawing::Image* image);

	int32_t getWidth() const;

	int32_t getHeight() const;

	drawing::Image* getImage() const;

private:
	Ref< drawing::Image > m_image;
};

	}
}

#endif	// traktor_flash_BitmapData_H
