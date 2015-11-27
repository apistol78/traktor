#ifndef traktor_drawing_ImageFormatTga_H
#define traktor_drawing_ImageFormatTga_H

#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief TGA format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatTga : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) T_OVERRIDE T_FINAL;

	virtual bool write(IStream* stream, Image* image) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_drawing_ImageFormatTga_H
