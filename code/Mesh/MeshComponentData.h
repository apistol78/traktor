#ifndef traktor_mesh_MeshComponentData_H
#define traktor_mesh_MeshComponentData_H

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
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

	namespace world
	{

class Entity;

	}

	namespace mesh
	{

class IMesh;
class MeshComponent;

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	MeshComponentData();

	Ref< MeshComponent > createComponent(world::Entity* owner, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< IMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< IMesh > m_mesh;
	bool m_screenSpaceCulling;
};

	}
}

#endif	// traktor_mesh_MeshComponentData_H
