#pragma once

#include "Render/Image2/ImagePassOpData.h"

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

/*!
 * \ingroup Render
 */
class T_DLLCLASS ShadowProjectData : public ImagePassOpData
{
    T_RTTI_CLASS;

public:
    virtual Ref< const ImagePassOp > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;
};

    }
}