#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

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
class ImagePass;
class ImageTargetSet;
class RenderGraph;
class RenderPass;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraph : public Object
{
    T_RTTI_CLASS;

public:
    void addTargetSets(RenderGraph* renderGraph) const;

    void addPasses(RenderGraph* renderGraph, RenderPass* parentPass, const ImageGraphContext& cx) const;

private:
    friend class ImageGraphData;

    RefArray< const ImageTargetSet > m_targetSets;
    RefArray< const ImagePass > m_passes;
    RefArray< const IImageStep > m_steps;
};

    }
}