#ifndef traktor_spark_Shape_H
#define traktor_spark_Shape_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Mesh;
class RenderContext;
class Shader;

	}

	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Shape : public Object
{
	T_RTTI_CLASS;

public:
	Shape(render::Mesh* mesh, const resource::Proxy< render::Shader >& shader);

	void render(render::RenderContext* renderContext, const Matrix33& transform) const;

private:
	Ref< render::Mesh > m_mesh;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_spark_Shape_H
