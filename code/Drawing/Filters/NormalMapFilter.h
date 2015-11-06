#ifndef traktor_drawing_NormalMapFilter_H
#define traktor_drawing_NormalMapFilter_H

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

/*! \brief Normal map image filter.
 * \ingroup Drawing
 *
 * Calculate a normal map by sampling heights from
 * source image.
 */
class T_DLLCLASS NormalMapFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	NormalMapFilter(float scale);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	float m_scale;
};
	
	}
}

#endif	// traktor_drawing_NormalMapFilter_H
