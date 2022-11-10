#pragma once

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

namespace traktor::drawing
{

/*! EXR format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatExr : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream) override final;

	virtual bool write(IStream* stream, const Image* image) override final;
};

}

#endif	// DRAWING_INCLUDE_EXR
