#ifndef traktor_mesh_SkinnedMeshResource_H
#define traktor_mesh_SkinnedMeshResource_H

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

class T_DLLCLASS SkinnedMeshResource : public IMeshResource
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
	) const;

	virtual void serialize(ISerializer& s);

private:
	friend class SkinnedMeshConverter;
	typedef std::list< Part > parts_t;

	resource::Id< render::Shader > m_shader;
	std::map< std::wstring, parts_t > m_parts;
	std::map< std::wstring, int32_t > m_jointMap;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshResource_H
