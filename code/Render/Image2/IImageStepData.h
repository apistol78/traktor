#pragma once

#include "Core/Ref.h"
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

class IImageStep;

/*!
 * \ingroup Render
 */
class T_DLLCLASS IImageStepData : public ISerializable
{
    T_RTTI_CLASS;

public:
    virtual Ref< const IImageStep > createInstance(resource::IResourceManager* resourceManager) const = 0;
};

    }
}