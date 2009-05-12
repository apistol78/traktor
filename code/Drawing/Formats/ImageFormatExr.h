#ifndef traktor_drawing_ImageFormatExr_H
#define traktor_drawing_ImageFormatExr_H

#include "Drawing/Config.h"
#include "Drawing/ImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_EXR)

namespace traktor
{
	namespace drawing
	{

/*! \brief EXR format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatExr : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatExr)

public:
	virtual Image* read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);
};

	}
}

#endif	// DRAWING_INCLUDE_EXR

#endif	// traktor_drawing_ImageFormatExr_H
