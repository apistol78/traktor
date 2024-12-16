/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class ImageGraph;
class ImagePassData;
class ImagePassStepData;
class ImageStructBufferData;
class ImageTargetSetData;
class ImageTextureData;
class IRenderSystem;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphData : public ISerializable
{
    T_RTTI_CLASS;

public:
    Ref< ImageGraph > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    struct PermutationData
    {
		uint32_t mask = 0;
		uint32_t value = 0;
		RefArray< ImageStructBufferData > sbuffers;
		RefArray< ImageTextureData > textures;
		RefArray< ImageTargetSetData > targetSets;
		RefArray< ImagePassData > passes;
		RefArray< ImagePassStepData > steps;

        void serialize(ISerializer& s);
    };

    std::wstring m_name;
    SmallMap< std::wstring, uint32_t > m_permutationBits;
    AlignedVector< PermutationData > m_permutations;
};

}
