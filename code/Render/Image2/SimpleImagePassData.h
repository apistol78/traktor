#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Render/Image2/IImagePassData.h"
#include "Resource/Id.h"

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
class T_DLLCLASS SimpleImagePassData : public IImagePassData
{
    T_RTTI_CLASS;

public:
    virtual Ref< IImagePass > createInstance(resource::IResourceManager* resourceManager) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    struct Source
    {
        std::wstring input;
        std::wstring parameter;

        void serialize(ISerializer& s);
    };

    resource::Id< Shader > m_shader;
    AlignedVector< Source > m_sources;
    std::wstring m_output;
};

    }
}