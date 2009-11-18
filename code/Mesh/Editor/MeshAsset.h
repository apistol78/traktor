#ifndef traktor_mesh_MeshAsset_H
#define traktor_mesh_MeshAsset_H

#include <map>
#include "Editor/Asset.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	enum MeshType
	{
		MtInvalid,
		MtBlend,
		MtIndoor,
		MtInstance,
		MtSkinned,
		MtStatic
	};

	MeshAsset();

	virtual const TypeInfo* getOutputType() const;

	virtual int getVersion() const;

	virtual bool serialize(ISerializer& s);

	void setMeshType(MeshType meshType) { m_meshType = meshType; }

	MeshType getMeshType() const { return m_meshType; }

	void setMaterialShaders(const std::map< std::wstring, Guid >& materialShaders) { m_materialShaders = materialShaders; }

	const std::map< std::wstring, Guid >& getMaterialShaders() const { return m_materialShaders; }

private:
	MeshType m_meshType;
	std::map< std::wstring, Guid > m_materialShaders;
};

	}
}

#endif	// traktor_mesh_MeshAsset_H
