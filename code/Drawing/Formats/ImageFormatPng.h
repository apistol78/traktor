#ifndef traktor_drawing_ImageFormatPng_H
#define traktor_drawing_ImageFormatPng_H

#include "Drawing/Config.h"
#include "Drawing/ImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_PNG)

namespace traktor
{
	namespace drawing
	{

/*! \brief PNG format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatPng : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatPng)

public:
	virtual Image* read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);
};

	}
}

#endif	// DRAWING_INCLUDE_PNG

#endif	// traktor_drawing_ImageFormatPng_H
