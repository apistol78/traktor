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

class IImagePass;
class RenderGraph;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraph : public Object
{
    T_RTTI_CLASS;

public:
    void setup(RenderGraph* renderGraph);

    void add(RenderGraph* renderGraph);

private:
    friend class ImageGraphData;

    RefArray< IImagePass > m_passes;
};

    }
}