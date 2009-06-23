#ifndef traktor_world_WorldEntityRenderers_H
#define traktor_world_WorldEntityRenderers_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityRenderer;

/*! \brief World entity renderers.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityRenderers : public Object
{
	T_RTTI_CLASS(WorldEntityRenderers)

public:
	typedef std::map< const Type*, EntityRenderer* > entity_renderer_map_t;

	void add(EntityRenderer* entityRenderer);

	void remove(EntityRenderer* entityRenderer);

	EntityRenderer* find(const Type& entityType) const;

	const RefArray< EntityRenderer >& get() const { return m_entityRenderers; }

private:
	RefArray< EntityRenderer > m_entityRenderers;
	entity_renderer_map_t m_entityRendererMap;
};

	}
}

#endif	// traktor_world_WorldEntityRenderers_H
