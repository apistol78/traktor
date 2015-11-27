#ifndef traktor_drawing_ImageFormatPng_H
#define traktor_drawing_ImageFormatPng_H

#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_PNG)

namespace traktor
{
	namespace drawing
	{

/*! \brief PNG format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatPng : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) T_OVERRIDE T_FINAL;

	virtual bool write(IStream* stream, Image* image) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// DRAWING_INCLUDE_PNG

#endif	// traktor_drawing_ImageFormatPng_H
