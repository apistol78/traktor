/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_LodMeshAsset_H
#define traktor_mesh_LodMeshAsset_H

#include <list>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{
	
/*! \brief
 */
class LodMeshAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	LodMeshAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class LodMeshPipeline;

	float m_maxDistance;
	float m_cullDistance;
	std::list< Guid > m_lods;
};

	}
}

#endif	// traktor_mesh_LodMeshAsset_H
