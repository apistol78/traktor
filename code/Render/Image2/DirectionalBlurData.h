#pragma once

#include "Core/Math/Vector2.h"
#include "Render/Image2/ImageStepData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

/*!
 * \ingroup Render
 */
class T_DLLCLASS DirectionalBlurData : public ImageStepData
{
    T_RTTI_CLASS;

public:
    DirectionalBlurData();

    virtual Ref< const ImageStep > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	enum BlurType
	{
		// 1D separable
		BtGaussian,
		BtSine,
		BtBox,
		// 2D combined
		BtBox2D,
		BtCircle2D
	};

	BlurType m_blurType;
	Vector2 m_direction;
	int32_t m_taps;
};

    }
}