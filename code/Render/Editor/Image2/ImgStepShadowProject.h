#pragma once

#include <list>
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

class T_DLLCLASS ImgStepShadowProject : public IImgStep
{
    T_RTTI_CLASS;

public:
    virtual std::wstring getTitle() const override final;

    virtual void getInputs(std::set< std::wstring >& outInputs) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    resource::Id< Shader > m_shader;
    std::list< std::wstring > m_parameters;
};

    }
}
