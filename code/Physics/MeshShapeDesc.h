#ifndef traktor_physics_MeshShapeDesc_H
#define traktor_physics_MeshShapeDesc_H

#include "Physics/ShapeDesc.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Mesh;

/*! \brief Mesh collision shape description.
 * \ingroup Physics
 */
class T_DLLCLASS MeshShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	void setMesh(const resource::Proxy< Mesh >& mesh);

	const resource::Proxy< Mesh >& getMesh() const;

	virtual bool serialize(ISerializer& s);

private:
	resource::Proxy< Mesh > m_mesh;
};

	}
}

#endif	// traktor_physics_MeshShapeDesc_H
