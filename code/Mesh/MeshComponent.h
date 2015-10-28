#ifndef traktor_mesh_MeshComponent_H
#define traktor_mesh_MeshComponent_H

#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	MeshComponent(world::Entity* owner, bool screenSpaceCulling);

	virtual void destroy() T_OVERRIDE;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

protected:
	world::Entity* m_owner;
	Ref< const IMeshParameterCallback > m_parameterCallback;
	bool m_screenSpaceCulling;
};

	}
}

#endif	// traktor_mesh_MeshComponent_H
