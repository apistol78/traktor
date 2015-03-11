#ifndef traktor_mesh_BlendMeshConverter_H
#define traktor_mesh_BlendMeshConverter_H

#include "Mesh/Editor/IMeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class BlendMeshConverter : public IMeshConverter
{
public:
	virtual Ref< IMeshResource > createResource() const;

	virtual bool convert(
		const MeshAsset* meshAsset,
		const RefArray< model::Model >& models,
		const model::Model* occluderModel,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const std::vector< render::VertexElement >& vertexElements,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const;
};

	}
}

#endif	// traktor_mesh_BlendMeshConverter_H
