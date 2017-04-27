/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_AutoLodMeshResource_H
#define traktor_mesh_AutoLodMeshResource_H

#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Mesh/IMeshResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class StaticMeshResource;

class T_DLLCLASS AutoLodMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	AutoLodMeshResource();

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class AutoLodMeshConverter;

	float m_maxDistance;
	float m_cullDistance;
	Aabb3 m_boundingBox;
	RefArray< StaticMeshResource > m_lods;
};

	}
}

#endif	// traktor_mesh_AutoLodMeshResource_H
