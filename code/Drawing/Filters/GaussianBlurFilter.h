#ifndef traktor_drawing_GaussianBlurFilter_H
#define traktor_drawing_GaussianBlurFilter_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Scalar.h"
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
	
/*! \brief Gaussian blur filter.
 * \ingroup Drawing
 */
class T_DLLCLASS GaussianBlurFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit GaussianBlurFilter(int32_t radius);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	AlignedVector< Scalar > m_kernel;
	int32_t m_size;
};
	
	}
}

#endif	// traktor_drawing_GaussianBlurFilter_H
