#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

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

class Entity;
class LightComponent;
class ProbeComponent;
class WorldEntityRenderers;

/*! World gather context.
 * \ingroup World
 */
class T_DLLCLASS WorldGatherContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldGatherContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity);

	void gather(const Object* renderable, AlignedVector< const LightComponent* >& outLights, AlignedVector< const ProbeComponent* >& outProbes) const;

	const Entity* getRootEntity() const { return m_rootEntity; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	const Entity* m_rootEntity;
};

	}
}

