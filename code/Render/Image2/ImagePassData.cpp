#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/IImageStepData.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImagePassData", 0, ImagePassData, ISerializable)

Ref< const ImagePass > ImagePassData::createInstance(resource::IResourceManager* resourceManager) const
{
    Ref< ImagePass > instance = new ImagePass();

    instance->m_output = getParameterHandle(m_output);

    for (auto stepData : m_steps)
    {
        Ref< const IImageStep > step = stepData->createInstance(resourceManager);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImagePassData::serialize(ISerializer& s)
{
    s >> Member< std::wstring >(L"output", m_output);
    s >> MemberRefArray< IImageStepData >(L"steps", m_steps);
}

    }
}
