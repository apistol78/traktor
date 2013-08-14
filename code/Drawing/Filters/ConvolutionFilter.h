#ifndef traktor_drawing_ConvolutionFilter_H
#define traktor_drawing_ConvolutionFilter_H

#include "Drawing/IImageFilter.h"
#include "Core/Math/Matrix33.h"

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
	
/*! \brief Convolution filter.
 * \ingroup Drawing
 */
class T_DLLCLASS ConvolutionFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	ConvolutionFilter(const Matrix33& matrix);

	static Ref< ConvolutionFilter > createGaussianBlur();

	static Ref< ConvolutionFilter > createEmboss();

protected:
	virtual void apply(Image* image) const;

private:
	Matrix33 m_matrix;
};
	
	}
}

#endif	// traktor_drawing_ConvolutionFilter_H
