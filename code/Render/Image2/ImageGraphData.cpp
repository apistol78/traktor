/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImagePassData.h"
#include "Render/Image2/ImagePassStepData.h"
#include "Render/Image2/ImageStructBufferData.h"
#include "Render/Image2/ImageTargetSetData.h"
#include "Render/Image2/ImageTextureData.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphData", 4, ImageGraphData, ISerializable)

Ref< ImageGraph > ImageGraphData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
    Ref< ImageGraph > instance = new ImageGraph(m_name);

    for (auto structBufferData : m_structBuffers)
    {
        Ref< const ImageStructBuffer > structBuffer = structBufferData->createInstance();
        if (!structBuffer)
            return nullptr;
        instance->m_structBuffers.push_back(structBuffer);
    }

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
        Ref< const ImagePassStep > step = stepData->createInstance(resourceManager, renderSystem);
        if (!step)
            return nullptr;
        instance->m_steps.push_back(step);
    }

    return instance;
}

void ImageGraphData::serialize(ISerializer& s)
{
    T_FATAL_ASSERT(s.getVersion< ImageGraphData >() >= 4);

    s >> Member< std::wstring >(L"name", m_name);
    s >> MemberRefArray< ImageStructBufferData >(L"structBuffers", m_structBuffers);
    s >> MemberRefArray< ImageTextureData >(L"textures", m_textures);
    s >> MemberRefArray< ImageTargetSetData >(L"targetSets", m_targetSets);
    s >> MemberRefArray< ImagePassData >(L"passes", m_passes);
    s >> MemberRefArray< ImagePassStepData >(L"steps", m_steps);
}

}
