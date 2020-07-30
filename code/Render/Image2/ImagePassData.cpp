#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImagePassOpData.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImagePassData", 0, ImagePassData, IImageStepData)

Ref< const IImageStep > ImagePassData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImagePass > instance = new ImagePass();

    instance->m_name = m_name;
    instance->m_outputTargetSet = m_outputTargetSet;

    for (auto opd : m_ops)
    {
        Ref< const ImagePassOp > op = opd->createInstance(resourceManager, renderSystem);
        if (!op)
            return nullptr;
        instance->m_ops.push_back(op);
    }

    return instance;
}

void ImagePassData::serialize(ISerializer& s)
{
    s >> Member< std::wstring >(L"name", m_name);
    s >> Member< int32_t >(L"outputTargetSet", m_outputTargetSet);
    s >> MemberRefArray< ImagePassOpData >(L"ops", m_ops);
}

    }
}
