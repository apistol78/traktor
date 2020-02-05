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

class ImageGraph;
class RenderContext;
class RenderGraph;
class RenderPass;

struct ImageGraphParams;

/*! Image pass step.
 * \ingroup Render
 * 
 * Each pass can have a sequence of steps
 * to implement it's functionality.
 */
class T_DLLCLASS IImageStep : public Object
{
    T_RTTI_CLASS;

public:
    /*! */
    virtual void setup(const ImageGraph* imageGraph, RenderPass& pass) const = 0;

    /*! */
    virtual void build(
        const ImageGraph* imageGraph,
        const RenderGraph& renderGraph,
        RenderContext* renderContext,
        const ImageGraphParams& data
    ) const = 0;
};

    }
}