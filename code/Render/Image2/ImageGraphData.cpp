#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/IImageStepData.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageTargetSetData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 0, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImageGraph > instance = new ImageGraph();

    for (auto targetSetData : m_targetSets)
    {
        Ref< const ImageTargetSet > targetSet = targetSetData->createInstance();
        if (!targetSet)
            return nullptr;
        instance->m_targetSets.push_back(targetSet);
    }

    for (auto passData : m_passes)
    {
        Ref< const ImagePass > pass = passData->createInstance(resourceManager, renderSystem);
        if (!pass)
            return nullptr;
        instance->m_passes.push_back(pass);
    }

    for (auto stepData : m_steps)
    {
        Ref< const IImageStep > step = stepData->createInstance(resourceManager, renderSystem);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    s >> MemberRefArray< ImageTargetSetData >(L"targetSets", m_targetSets);
    s >> MemberRefArray< ImagePassData >(L"passes", m_passes);
    s >> MemberRefArray< IImageStepData >(L"steps", m_steps);
}

    }
}