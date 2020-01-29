#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Image2/IImagePass.h"
#include "Resource/Proxy.h"

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

class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS SimpleImagePass : public IImagePass
{
    T_RTTI_CLASS;

public:
    SimpleImagePass();
    
    virtual void setup(RenderGraph* renderGraph) const override final;

    virtual void add(RenderGraph* renderGraph) const override final;

private:
    friend class SimpleImagePassData;

    struct Source
    {
        handle_t input;
        handle_t parameter;
    };

    resource::Proxy< render::Shader > m_shader;
    AlignedVector< Source > m_sources;
    handle_t m_output;
};

    }
}