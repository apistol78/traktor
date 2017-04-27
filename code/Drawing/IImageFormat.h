/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_IImageFormat_H
#define traktor_drawing_IImageFormat_H

#include "Core/Object.h"
#include "Core/Io/Path.h"
#include "Core/Io/IStream.h"

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

class Image;

/*! \brief Image file format base class.
 * \ingroup Drawing
 */
class T_DLLCLASS IImageFormat : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< IImageFormat > determineFormat(const std::wstring& extension);

	static Ref< IImageFormat > determineFormat(const Path& fileName);

	virtual Ref< Image > read(IStream* stream) = 0;

	virtual bool write(IStream* stream, Image* image) = 0;
};

	}
}

#endif	// traktor_drawing_IImageFormat_H
