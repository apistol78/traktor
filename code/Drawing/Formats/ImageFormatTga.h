#ifndef traktor_drawing_ImageFormatTga_H
#define traktor_drawing_ImageFormatTga_H

#include "Drawing/ImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief TGA format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatTga : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatTga)

public:
	virtual Ref< Image > read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);
};

	}
}

#endif	// traktor_drawing_ImageFormatTga_H
