#pragma once

#include "Core/Object.h"

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

class RenderGraph;

/*!
 * \ingroup Render
 */
class T_DLLCLASS IImagePass : public Object
{
    T_RTTI_CLASS;

public:
    /*! Setup transient targets in render graph. */
    virtual void setup(RenderGraph* renderGraph) const = 0;

    /*! Add pass to render graph. */
    virtual void add(RenderGraph* renderGraph) const = 0;	
};

    }
}