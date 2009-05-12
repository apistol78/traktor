#ifndef traktor_mesh_MeshPipelineParams_H
#define traktor_mesh_MeshPipelineParams_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Guid.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace model
	{

class Model;

	}

	namespace render
	{

class ShaderGraph;

	}

	namespace mesh
	{

class MeshPipelineParams : public Object
{
	T_RTTI_CLASS(MeshPipelineParams)

public:
	struct MaterialInfo
	{
		Guid guid;
		Ref< const render::ShaderGraph > graph;
	};

	Ref< model::Model > m_model;
	std::map< std::wstring, MaterialInfo > m_materialMap;
};

	}
}

#endif	// traktor_mesh_MeshPipelineParams_H
