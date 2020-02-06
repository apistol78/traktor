#pragma once

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
    virtual void serialize(ISerializer& s) override final;

private:
    resource::Id< Shader > m_shader;
};

    }
}
