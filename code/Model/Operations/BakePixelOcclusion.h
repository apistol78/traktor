#ifndef traktor_model_BakePixelOcclusion_H
#define traktor_model_BakePixelOcclusion_H

#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace model
	{

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS BakePixelOcclusion : public IModelOperation
{
	T_RTTI_CLASS;

public:
	BakePixelOcclusion(
		drawing::Image* occlusionImage,
		int32_t rayCount = 64,
		float raySpread = 0.75f,
		float rayBias = 0.1f
	);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	Ref< drawing::Image > m_occlusionImage;
	int32_t m_rayCount;
	float m_raySpread;
	float m_rayBias;
};

	}
}

#endif	// traktor_model_BakePixelOcclusion_H
