#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Shape", Shape, Character)

Shape::Shape(render::Mesh* mesh, const resource::Proxy< render::Shader >& shader)
:	m_mesh(mesh)
,	m_shader(shader)
{
}

Ref< CharacterInstance > Shape::createInstance() const
{
	return new ShapeInstance(this);
}

	}
}
