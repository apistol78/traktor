/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_MeshShapeDesc_H
#define traktor_physics_MeshShapeDesc_H

#include "Physics/ShapeDesc.h"
#include "Resource/Id.h"

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
	void setMesh(const resource::Id< Mesh >& mesh);

	const resource::Id< Mesh >& getMesh() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	resource::Id< Mesh > m_mesh;
};

	}
}

#endif	// traktor_physics_MeshShapeDesc_H
