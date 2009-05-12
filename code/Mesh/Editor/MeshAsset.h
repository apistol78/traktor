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
	T_RTTI_CLASS(MeshAsset)

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

	virtual const Type* getOutputType() const;

	virtual bool serialize(Serializer& s);

private:
	friend class MeshAssetWizardTool;
	friend class MeshPipeline;

	MeshType m_meshType;
};

	}
}

#endif	// traktor_mesh_MeshAsset_H
