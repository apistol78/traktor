#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Render/Image2/IImageStepData.h"

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

class ImagePassOpData;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePassData : public IImageStepData
{
    T_RTTI_CLASS;

public:
    virtual Ref< const IImageStep > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    std::wstring m_name;
    int32_t m_outputTargetSet;
    Clear m_clear;
    RefArray< ImagePassOpData > m_ops;
};

    }
}
