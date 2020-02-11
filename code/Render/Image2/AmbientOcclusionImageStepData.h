#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Render/Image2/IImageStepData.h"
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
class T_DLLCLASS AmbientOcclusionImageStepData : public IImageStepData
{
    T_RTTI_CLASS;

public:
    virtual Ref< const IImageStep > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    struct Source
    {
        std::wstring textureId;
        std::wstring parameter;

        void serialize(ISerializer& s);
    };

    resource::Id< Shader > m_shader;
    AlignedVector< Source > m_sources;
};

    }
}