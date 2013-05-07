#ifndef traktor_ai_NavMeshEntity_H
#define traktor_ai_NavMeshEntity_H

#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

class NavMesh;

class T_DLLCLASS NavMeshEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	NavMeshEntity(const resource::Proxy< NavMesh >& navMesh);

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

	const resource::Proxy< NavMesh >& get() const { return m_navMesh; }

private:
	resource::Proxy< NavMesh > m_navMesh;
};

	}
}

#endif	// traktor_ai_NavMeshEntity_H
