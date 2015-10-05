#ifndef traktor_mesh_IndoorMeshResource_H
#define traktor_mesh_IndoorMeshResource_H

#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
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

class T_DLLCLASS IndoorMeshResource : public IMeshResource
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

	typedef std::list< Part > parts_t;

	struct Sector
	{
		Vector4 min;
		Vector4 max;
		std::map< std::wstring, parts_t > parts;

		void serialize(ISerializer& s);
	};

	struct Portal
	{
		AlignedVector< Vector4 > pts;
		int32_t sectorA;
		int32_t sectorB;

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
	friend class IndoorMeshConverter;

	resource::Id< render::Shader > m_shader;
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;
};

	}
}

#endif	// traktor_mesh_IndoorMeshResource_H
