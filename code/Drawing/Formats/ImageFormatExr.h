/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ImageFormatExr_H
#define traktor_drawing_ImageFormatExr_H

#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_EXR)

namespace traktor
{
	namespace drawing
	{

/*! \brief EXR format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatExr : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream);

	virtual bool write(IStream* stream, Image* image);
};

	}
}

#endif	// DRAWING_INCLUDE_EXR

#endif	// traktor_drawing_ImageFormatExr_H
