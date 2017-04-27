/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_BlendMeshResource_H
#define traktor_mesh_BlendMeshResource_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Mesh/IMeshResource.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace mesh
	{

class T_DLLCLASS BlendMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart;

		Part();

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class BlendMeshConverter;
	typedef std::list< Part > parts_t;

	resource::Id< render::Shader > m_shader;
	std::map< std::wstring, parts_t > m_parts;
	std::map< std::wstring, int > m_targetMap;
};

	}
}

#endif	// traktor_mesh_BlendMeshResource_H
