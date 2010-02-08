#ifndef traktor_mesh_BlendMeshConverter_H
#define traktor_mesh_BlendMeshConverter_H

#include "Mesh/Editor/MeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class BlendMeshConverter : public MeshConverter
{
public:
	virtual Ref< MeshResource > createResource() const;

	virtual bool convert(
		const RefArray< model::Model >& models,
		const std::map< std::wstring, MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		MeshResource* meshResource,
		IStream* meshResourceStream
	) const;
};

	}
}

#endif	// traktor_mesh_BlendMeshConverter_H
