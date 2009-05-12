#ifndef traktor_mesh_MeshConverter_H
#define traktor_mesh_MeshConverter_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Guid.h"
#include "Render/VertexElement.h"
#include "Mesh/Editor/MeshPipelineParams.h"

namespace traktor
{

class Stream;

	namespace model
	{

class Model;

	}

	namespace mesh
	{

class MeshResource;

class MeshConverter : public Object
{
public:
	virtual MeshResource* createResource() const = 0;

	virtual bool convert(
		const model::Model& model,
		const std::map< std::wstring, MeshPipelineParams::MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		MeshResource* meshResource,
		Stream* meshResourceStream
	) const = 0;
};

	}
}

#endif	// traktor_mesh_MeshConverter_H
