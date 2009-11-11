#ifndef traktor_drawing_ImageFormatJpeg_H
#define traktor_drawing_ImageFormatJpeg_H

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

class ImageFormatJpegImpl;

/*! \brief JPEG format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatJpeg : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatJpeg)

public:
	ImageFormatJpeg();

	virtual Ref< Image > read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);

	bool readJpegHeader(Stream* stream);

	Ref< Image > readJpegImage(Stream* stream);

private:
	ImageFormatJpegImpl* m_impl;
};

	}
}

#endif	// traktor_drawing_ImageFormatJpeg_H
