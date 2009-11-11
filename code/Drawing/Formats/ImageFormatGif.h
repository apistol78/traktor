#ifndef traktor_drawing_ImageFormatGif_H
#define traktor_drawing_ImageFormatGif_H

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

/*! \brief GIF format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatGif : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatGif)

public:
	virtual Ref< Image > read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);
};

	}
}

#endif	// traktor_drawing_ImageFormatGif_H
