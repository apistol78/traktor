#pragma once

#include <list>
#include "Render/Editor/Image2/IImgStep.h"
#include "Resource/Id.h"

namespace traktor
{
    namespace render
    {

class Shader;

class T_DLLCLASS ImgStepSimple : public IImgStep
{
    T_RTTI_CLASS;

public:
    virtual void getInputs(std::set< std::wstring >& outInputs) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    resource::Id< Shader > m_shader;
    std::list< std::wstring > m_parameters;
};

    }
}
