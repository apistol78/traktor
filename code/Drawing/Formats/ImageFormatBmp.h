/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ImageFormatBmp_H
#define traktor_drawing_ImageFormatBmp_H

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

/*! \brief BMP format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatBmp : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) T_OVERRIDE T_FINAL;

	virtual bool write(IStream* stream, Image* image) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_drawing_ImageFormatBmp_H
