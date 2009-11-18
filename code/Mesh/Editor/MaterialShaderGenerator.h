#ifndef traktor_mesh_MaterialShaderGenerator_H
#define traktor_mesh_MaterialShaderGenerator_H

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace render
	{

class ShaderGraph;

	}

	namespace model
	{

class Model;
class Material;

	}

	namespace mesh
	{

class MaterialShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	MaterialShaderGenerator(db::Database* database, const model::Model* model);

	Ref< render::ShaderGraph > generate(const model::Material& material) const;

private:
	Ref< db::Database > m_database;
	Ref< const model::Model > m_model;
};

	}
}

#endif	// traktor_mesh_MaterialShaderGenerator_H
