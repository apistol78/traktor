#ifndef traktor_world_WorldEntityRenderers_H
#define traktor_world_WorldEntityRenderers_H

#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"

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

class IEntityRenderer;

/*! \brief World entity renderers.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityRenderers : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::map< const TypeInfo*, IEntityRenderer* > entity_renderer_map_t;

	void add(IEntityRenderer* entityRenderer);

	void remove(IEntityRenderer* entityRenderer);

	IEntityRenderer* find(const TypeInfo& entityType) const;

	const RefArray< IEntityRenderer >& get() const { return m_entityRenderers; }

private:
	RefArray< IEntityRenderer > m_entityRenderers;
	entity_renderer_map_t m_entityRendererMap;
};

	}
}

#endif	// traktor_world_WorldEntityRenderers_H
