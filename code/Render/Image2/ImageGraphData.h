#pragma once

#include "Core/Ref.h"
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

class IImageStepData;
class ImageGraph;
class ImagePassData;
class ImageTargetSetData;
class IRenderSystem;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphData : public ISerializable
{
    T_RTTI_CLASS;

public:
    Ref< ImageGraph > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    RefArray< ImageTargetSetData > m_targetSets;
    RefArray< ImagePassData > m_passes;
    RefArray< IImageStepData > m_steps;
};

    }
}