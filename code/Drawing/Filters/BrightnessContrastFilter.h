#ifndef traktor_drawing_BrightnessContrastFilter_H
#define traktor_drawing_BrightnessContrastFilter_H

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
	
/*! \brief Brightness & Contrast filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BrightnessContrastFilter : public ImageFilter
{
	T_RTTI_CLASS(BrightnessContrastFilter)

public:
	BrightnessContrastFilter(float brightness, float contrast);

protected:
	virtual Image* apply(const Image* image);

private:
	float m_brightness;
	float m_contrast;
};
	
	}
}

#endif	// traktor_drawing_BrightnessContrastFilter_H
