#pragma once

#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class ImageFormatJpegImpl;

/*! JPEG format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatJpeg : public IImageFormat
{
	T_RTTI_CLASS;

public:
	ImageFormatJpeg();

	virtual ~ImageFormatJpeg();

	virtual Ref< Image > read(IStream* stream) override final;

	virtual bool write(IStream* stream, const Image* image) override final;

	bool readJpegHeader(IStream* stream);

	Ref< Image > readJpegImage(IStream* stream);

private:
	ImageFormatJpegImpl* m_impl;
};

}
