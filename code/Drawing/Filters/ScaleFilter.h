#ifndef traktor_drawing_ScaleFilter_H
#define traktor_drawing_ScaleFilter_H

#include "Drawing/ImageFilter.h"

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

/*! \brief Scale image filter.
 * \ingroup Drawing
 *
 * Magnify or minify image, either using point sampling or
 * linear filtering.
 */
class T_DLLCLASS ScaleFilter : public ImageFilter
{
	T_RTTI_CLASS(ScaleFilter)

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
		uint32_t width,
		uint32_t height,
		MinifyType minify,
		MagnifyType magnify,
		bool keepZeroAlpha = false
	);

protected:
	virtual Image* apply(const Image* image);

private:
	uint32_t m_width;
	uint32_t m_height;
	MinifyType m_minify;
	MagnifyType m_magnify;
	bool m_keepZeroAlpha;
};
	
	}
}

#endif	// traktor_drawing_ScaleFilter_H
