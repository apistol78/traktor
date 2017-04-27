/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ScaleFilter_H
#define traktor_drawing_ScaleFilter_H

#include "Drawing/IImageFilter.h"

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

/*! \brief Scale image filter.
 * \ingroup Drawing
 *
 * Magnify or minify image, either using point sampling or
 * linear filtering.
 */
class T_DLLCLASS ScaleFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	// Scale method when image is scaled down.
	enum MinifyType
	{
		MnCenter,	// Center source pixel.
		MnAverage	// Average pixels from source rectangle.
	};

	// Scale method when image is scaled up.
	enum MagnifyType
	{
		MgNearest,	// Nearest source pixel.
		MgLinear	// Linear interpolate source pixels.
	};

	ScaleFilter(
		int32_t width,
		int32_t height,
		MinifyType minify,
		MagnifyType magnify,
		bool keepZeroAlpha = false
	);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	int32_t m_width;
	int32_t m_height;
	MinifyType m_minify;
	MagnifyType m_magnify;
	bool m_keepZeroAlpha;
};
	
	}
}

#endif	// traktor_drawing_ScaleFilter_H
