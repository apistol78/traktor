#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImageStepData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ImageTextureData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 0, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImageGraph > instance = new ImageGraph();

    for (auto textureData : m_textures)
    {
        Ref< const ImageTexture > texture = textureData->createInstance(resourceManager);
        if (!texture)
            return nullptr;
        instance->m_textures.push_back(texture);
    }

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
        Ref< const ImageStep > step = stepData->createInstance(resourceManager, renderSystem);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    s >> MemberRefArray< ImageTextureData >(L"textures", m_textures);
    s >> MemberRefArray< ImageTargetSetData >(L"targetSets", m_targetSets);
    s >> MemberRefArray< ImagePassData >(L"passes", m_passes);
    s >> MemberRefArray< ImageStepData >(L"steps", m_steps);
}

    }
}