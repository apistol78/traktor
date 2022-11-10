/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class IImageStep;
class ImageGraphContext;
class ImagePassOp;
class ImageTargetSet;
class ImageTexture;
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
	    const ImageGraphView& view
    ) const;

private:
    friend class ImageGraphData;

    std::wstring m_name;
    RefArray< const ImageTexture > m_textures;
    RefArray< const ImageTargetSet > m_targetSets;
    RefArray< const IImageStep > m_steps;
    RefArray< const ImagePassOp > m_ops;
};

    }
}