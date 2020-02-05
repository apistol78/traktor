#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/IImageStepData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 0, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager) const
{
    Ref< ImageGraph > instance = new ImageGraph();

    for (auto passData : m_passes)
    {
        Ref< const ImagePass > pass = passData->createInstance(resourceManager);
        if (!pass)
            return nullptr;
        instance->m_passes.push_back(pass);
    }

    for (auto stepData : m_steps)
    {
        Ref< const IImageStep > step = stepData->createInstance(resourceManager);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    s >> MemberRefArray< ImagePassData >(L"passes", m_passes);
    s >> MemberRefArray< IImageStepData >(L"steps", m_steps);
}

    }
}