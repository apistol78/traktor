#ifndef traktor_mesh_MaterialShaderGenerator_H
#define traktor_mesh_MaterialShaderGenerator_H

#include "Core/Object.h"

namespace traktor
{

class PropertyGroup;

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
	MaterialShaderGenerator(const PropertyGroup* templates);

	Ref< render::ShaderGraph > generate(db::Database* database, const model::Material& material, const std::map< std::wstring, Guid >& textures) const;
	
	void addDependencies(editor::IPipelineDepends* pipelineDepends);

private:
	Ref< const PropertyGroup > m_templates;
};

	}
}

#endif	// traktor_mesh_MaterialShaderGenerator_H
