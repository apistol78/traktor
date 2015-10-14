#ifndef traktor_spark_Shape_H
#define traktor_spark_Shape_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
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

class Matrix33;

	namespace render
	{

class ITexture;
class Mesh;
class RenderContext;
class Shader;

	}

	namespace spark
	{

class ColorTransform;

/*! \brief Pre-triangulated renderable shape.
 * \ingroup Spark
 */
class T_DLLCLASS Shape : public Object
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		resource::Proxy< render::ITexture > texture;
		Color4f fillColor;
		int8_t curveSign;
	};

	Shape(
		render::Mesh* mesh,
		const resource::Proxy< render::Shader >& shader,
		const AlignedVector< Part >& parts,
		const Aabb2& bounds
	);

	const Aabb2& getBounds() const { return m_bounds; }

	void render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const;

private:
	Ref< render::Mesh > m_mesh;
	mutable resource::Proxy< render::Shader > m_shader;
	AlignedVector< Part > m_parts;
	Aabb2 m_bounds;
};

	}
}

#endif	// traktor_spark_Shape_H
