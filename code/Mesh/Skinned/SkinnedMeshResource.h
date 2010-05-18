#ifndef traktor_mesh_SkinnedMeshResource_H
#define traktor_mesh_SkinnedMeshResource_H

#include <map>
#include "Core/Guid.h"
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

class T_DLLCLASS SkinnedMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart;
		bool opaque;

		Part();

		bool serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class SkinnedMeshConverter;
	typedef std::list< Part > parts_t;

	Guid m_shader;
	std::map< std::wstring, parts_t > m_parts;
	std::map< std::wstring, int > m_boneMap;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshResource_H
