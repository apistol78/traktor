#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/IImagePassData.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImageGraphData", 0, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager) const
{
    Ref< ImageGraph > instance = new ImageGraph();
    for (auto passData : m_passes)
    {
        Ref< IImagePass > pass = passData->createInstance(resourceManager);
        if (!pass)
            return nullptr;

        instance->m_passes.push_back(pass);
    }
    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    s >> MemberRefArray< IImagePassData >(L"passes", m_passes);
}

    }
}