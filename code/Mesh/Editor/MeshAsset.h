#ifndef traktor_mesh_MeshAsset_H
#define traktor_mesh_MeshAsset_H

#include <map>
#include "Core/Guid.h"
#include "Editor/Asset.h"

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
		MtPartition,
		MtSkinned,
		MtStatic,
		MtStream
	};

	MeshAsset();

	virtual void serialize(ISerializer& s);

	void setMeshType(MeshType meshType) { m_meshType = meshType; }

	MeshType getMeshType() const { return m_meshType; }

	void setMaterialTemplates(const std::map< std::wstring, Guid >& materialTemplates) { m_materialTemplates = materialTemplates; }

	const std::map< std::wstring, Guid >& getMaterialTemplates() const { return m_materialTemplates; }

	void setMaterialShaders(const std::map< std::wstring, Guid >& materialShaders) { m_materialShaders = materialShaders; }

	const std::map< std::wstring, Guid >& getMaterialShaders() const { return m_materialShaders; }

	void setMaterialTextures(const std::map< std::wstring, Guid >& materialTextures) { m_materialTextures = materialTextures; }

	const std::map< std::wstring, Guid >& getMaterialTextures() const { return m_materialTextures; }
	
	void setBakeOcclusion(bool bakeOcclusion) { m_bakeOcclusion = bakeOcclusion; }
	
	bool getBakeOcclusion() const { return m_bakeOcclusion; }

	void setCullDistantFaces(bool cullDistantFaces) { m_cullDistantFaces = cullDistantFaces; }

	bool getCullDistantFaces() const { return m_cullDistantFaces; }

	void setGenerateOccluder(bool generateOccluder) { m_generateOccluder = generateOccluder; }

	bool getGenerateOccluder() const { return m_generateOccluder; }

	void setAutoDetailLevel(float autoDetailLevel) { m_autoDetailLevel = autoDetailLevel; }

	float getAutoDetailLevel() const { return m_autoDetailLevel; }

private:
	MeshType m_meshType;
	std::map< std::wstring, Guid > m_materialTemplates;
	std::map< std::wstring, Guid > m_materialShaders;
	std::map< std::wstring, Guid > m_materialTextures;
	bool m_bakeOcclusion;
	bool m_cullDistantFaces;
	bool m_generateOccluder;
	float m_autoDetailLevel;
};

	}
}

#endif	// traktor_mesh_MeshAsset_H
