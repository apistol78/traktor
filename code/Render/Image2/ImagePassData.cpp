#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageStepData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImagePassData", 0, ImagePassData, ISerializable)

Ref< const ImagePass > ImagePassData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImagePass > instance = new ImagePass();

    instance->m_name = m_name;
    instance->m_outputTargetSet = m_outputTargetSet;

    for (auto stepData : m_steps)
    {
        Ref< const ImageStep > step = stepData->createInstance(resourceManager, renderSystem);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImagePassData::serialize(ISerializer& s)
{
    s >> Member< std::wstring >(L"name", m_name);
    s >> Member< int32_t >(L"outputTargetSet", m_outputTargetSet);
    s >> MemberRefArray< ImageStepData >(L"steps", m_steps);
}

    }
}
