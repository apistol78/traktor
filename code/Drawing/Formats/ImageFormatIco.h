/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ImageFormatIco_H
#define traktor_drawing_ImageFormatIco_H

#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(DRAWING_INCLUDE_ICO)

namespace traktor
{
	namespace drawing
	{

/*! \brief ICO format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatIco : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) T_OVERRIDE T_FINAL;

	virtual bool write(IStream* stream, Image* image) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// DRAWING_INCLUDE_ICO

#endif	// traktor_drawing_ImageFormatIco_H
