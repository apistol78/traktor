#ifndef traktor_drawing_BrightnessContrastFilter_H
#define traktor_drawing_BrightnessContrastFilter_H

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
	
/*! \brief Brightness & Contrast filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BrightnessContrastFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	BrightnessContrastFilter(float brightness, float contrast);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	float m_brightness;
	float m_contrast;
};
	
	}
}

#endif	// traktor_drawing_BrightnessContrastFilter_H
