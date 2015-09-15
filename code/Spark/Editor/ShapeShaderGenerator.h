#ifndef traktor_spark_ShapeShaderGenerator_H
#define traktor_spark_ShapeShaderGenerator_H

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

	namespace spark
	{

class ShapeShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	Ref< render::ShaderGraph > generate(db::Database* database, const Guid& outputFragment) const;
	
	void addDependencies(editor::IPipelineDepends* pipelineDepends);
};

	}
}

#endif	// traktor_spark_ShapeShaderGenerator_H
