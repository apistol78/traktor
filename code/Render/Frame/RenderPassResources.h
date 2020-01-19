#pragma once

#include "Core/Object.h"
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

class IRenderTargetSet;
class RenderGraph;
class RenderPass;

class T_DLLCLASS RenderPassResources : public Object
{
    T_RTTI_CLASS;

public:
    explicit RenderPassResources(const RenderGraph* renderGraph, const RenderPass& pass);

	IRenderTargetSet* getInput(handle_t targetId) const;

private:
    const RenderGraph* m_renderGraph;
    const RenderPass& m_pass;
};

    }
}