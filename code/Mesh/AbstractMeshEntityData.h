/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_AbstractMeshEntityData_H
#define traktor_mesh_AbstractMeshEntityData_H

#include "Core/Math/Matrix44.h"
#include "World/EntityData.h"

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

class IEntityBuilder;

	}

	namespace mesh
	{

class MeshEntity;

/*! \brief Abstract mesh entity data.
 * \ingroup Mesh
 */
class T_DLLCLASS AbstractMeshEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const = 0;
};

	}
}

#endif	// traktor_mesh_AbstractMeshEntityData_H
