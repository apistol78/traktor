#ifndef traktor_mesh_ShaderBuilder_H
#define traktor_mesh_ShaderBuilder_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

class Database;
class Group;

	}

	namespace model
	{

class Material;

	}

	namespace mesh
	{

class ShaderBuilder : public Object
{
	T_RTTI_CLASS(ShaderBuilder)

public:
	ShaderBuilder(
		db::Database* db,
		db::Group* shaderGroup
	);

	Guid build(const model::Material& material);

private:
	Ref< db::Database > m_db;
	Ref< db::Group > m_shaderGroup;
};

	}
}

#endif	// traktor_mesh_ShaderBuilder_H
