#pragma once

#include <list>
#include "Core/Math/Vector2.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class Shader;

class T_DLLCLASS ImgStepDirectionalBlur : public IImgStep
{
    T_RTTI_CLASS;

public:
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

	ImgStepDirectionalBlur();

    virtual std::wstring getTitle() const override final;

    virtual void getInputs(std::set< std::wstring >& outInputs) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	BlurType m_blurType;
	Vector2 m_direction;
	int32_t m_taps;
    resource::Id< Shader > m_shader;
    std::list< std::wstring > m_parameters;
};

    }
}
