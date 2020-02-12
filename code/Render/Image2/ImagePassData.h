#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace resource
    {

class IResourceManager;

    }

    namespace render
    {

class ImagePass;
class ImageStepData;
class IRenderSystem;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePassData : public ISerializable
{
    T_RTTI_CLASS;

public:
    Ref< const ImagePass > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    std::wstring m_output;
    RefArray< ImageStepData > m_steps;
};

    }
}
