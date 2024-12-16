/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ImageGraphContext;
class ImagePass;
class ImagePassStep;
class ImageStructBuffer;
class ImageTargetSet;
class ImageTexture;
class ProgramParameters;
class RenderGraph;
class RenderPass;
class ScreenRenderer;

struct ImageGraphView;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraph : public Object
{
    T_RTTI_CLASS;

public:
    explicit ImageGraph(const std::wstring& name);

    void addPasses(
	    ScreenRenderer* screenRenderer,
	    RenderGraph& renderGraph,
	    RenderPass* pass,
	    const ImageGraphContext& cx,
	    const ImageGraphView& view,
        const std::function< void(const RenderGraph& renderGraph, ProgramParameters*) >& parametersFn
    ) const;

    void setPermutation(handle_t handle, bool param, uint32_t& inoutPermutationValue) const;

private:
    friend class ImageGraphData;

    struct Permutation
    {
        uint32_t mask;
        uint32_t value;
        RefArray< const ImageStructBuffer > sbuffers;
        RefArray< const ImageTexture > textures;
        RefArray< const ImageTargetSet > targetSets;
        RefArray< const ImagePass > passes;
        RefArray< const ImagePassStep > steps;
    };

    std::wstring m_name;
    SmallMap< handle_t, uint32_t > m_permutationBits;
    AlignedVector< Permutation > m_permutations;
};

}
