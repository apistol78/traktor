#ifndef traktor_mesh_StaticMeshConverter_H
#define traktor_mesh_StaticMeshConverter_H

#include "Mesh/Editor/MeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class StaticMeshConverter : public MeshConverter
{
public:
	virtual Ref< MeshResource > createResource() const;

	virtual bool convert(
		const model::Model& model,
		const std::map< std::wstring, MeshPipelineParams::MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		MeshResource* meshResource,
		IStream* meshResourceStream
	) const;
};

	}
}

#endif	// traktor_mesh_StaticMeshConverter_H
