#ifndef traktor_mesh_IndoorMeshConverter_H
#define traktor_mesh_IndoorMeshConverter_H

#include "Mesh/Editor/IMeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class IndoorMeshConverter : public IMeshConverter
{
public:
	virtual Ref< IMeshResource > createResource() const T_OVERRIDE T_FINAL;

	virtual bool convert(
		const MeshAsset* meshAsset,
		const RefArray< model::Model >& models,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const std::vector< render::VertexElement >& vertexElements,
		int32_t maxInstanceCount,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_mesh_IndoorMeshConverter_H
