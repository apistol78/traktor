/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Navigation mesh entity.
 * \ingroup AI
 *
 * Navigation mesh entities are used to attach a navigation mesh
 * to a scene and thus being accessible through-out the engine.
 */
class T_DLLCLASS NavMeshEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	NavMeshEntity();

	NavMeshEntity(const resource::Proxy< NavMesh >& navMesh);

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	/*! \brief Get navigation mesh.
	 *
	 * \return Navigation mesh proxy.
	 */
	const resource::Proxy< NavMesh >& get() const { return m_navMesh; }

private:
	resource::Proxy< NavMesh > m_navMesh;
};

	}
}

#endif	// traktor_ai_NavMeshEntity_H
