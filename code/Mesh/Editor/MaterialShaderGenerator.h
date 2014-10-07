#ifndef traktor_mesh_MaterialShaderGenerator_H
#define traktor_mesh_MaterialShaderGenerator_H

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class Database;

	}
	
	namespace editor
	{
	
class IPipelineDepends;
	
	}

	namespace render
	{

class ShaderGraph;

	}

	namespace model
	{

class Material;

	}

	namespace mesh
	{

class MaterialShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	Ref< render::ShaderGraph > generate(
		db::Database* database,
		const model::Material& material,
		const Guid& materialTemplate,
		const std::map< std::wstring, Guid >& textures,
		bool vertexColor
	) const;
	
	void addDependencies(editor::IPipelineDepends* pipelineDepends);
};

	}
}

#endif	// traktor_mesh_MaterialShaderGenerator_H
