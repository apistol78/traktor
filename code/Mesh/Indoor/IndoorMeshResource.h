#ifndef traktor_mesh_IndoorMeshResource_H
#define traktor_mesh_IndoorMeshResource_H

#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb.h"
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

class T_DLLCLASS IndoorMeshResource : public IMeshResource
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

	typedef std::list< Part > parts_t;

	struct Sector
	{
		Vector4 min;
		Vector4 max;
		std::map< std::wstring, parts_t > parts;

		bool serialize(ISerializer& s);
	};

	struct Portal
	{
		AlignedVector< Vector4 > pts;
		int32_t sectorA;
		int32_t sectorB;

		bool serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class IndoorMeshConverter;

	Guid m_shader;
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;
};

	}
}

#endif	// traktor_mesh_IndoorMeshResource_H
