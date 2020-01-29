#include "Render/Image2/IImagePass.h"
#include "Render/Image2/ImageGraph.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraph", ImageGraph, Object)

void ImageGraph::setup(RenderGraph* renderGraph)
{
    for (auto pass : m_passes)
        pass->setup(renderGraph);
}

void ImageGraph::add(RenderGraph* renderGraph)
{
    for (auto pass : m_passes)
        pass->add(renderGraph);
}

    }
}