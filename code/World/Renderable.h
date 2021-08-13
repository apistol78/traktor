#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace world
    {

class IEntityRenderer;
class WorldEntityRenderers;

/*! Renderable object.
 * \ingroup World
 */
class T_DLLCLASS Renderable : public Object
{
    T_RTTI_CLASS;

private:
    friend class WorldBuildContext;
    friend class WorldGatherContext;
    friend class WorldSetupContext;

    mutable const WorldEntityRenderers* m_entityRenderers = nullptr;
    mutable IEntityRenderer* m_entityRenderer = nullptr;
};

    }
}