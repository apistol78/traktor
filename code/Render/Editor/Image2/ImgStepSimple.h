#pragma once

#include "Core/Containers/AlignedVector.h"
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
    struct Source
    {
        std::wstring parameter;
        std::wstring targetSetId;
        int32_t colorIndex;

        Source();

        void serialize(ISerializer& s);
    };

    resource::Id< Shader > m_shader;
    AlignedVector< Source > m_sources;
};

    }
}
