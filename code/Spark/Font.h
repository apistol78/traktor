#ifndef traktor_spark_Font_H
#define traktor_spark_Font_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector4.h"
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

class ISimpleTexture;
class RenderContext;
class Shader;
class VertexBuffer;

	}

	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Font : public Object
{
	T_RTTI_CLASS;

public:
	float advance(uint32_t character) const;

	void render(render::RenderContext* renderContext, const Matrix33& transform, uint32_t character, float height) const;

private:
	friend class FontResource;

	struct Glyph
	{
		Vector4 rect;
		Vector4 unit;
		float advance;
	};

	mutable resource::Proxy< render::ISimpleTexture > m_texture;
	mutable resource::Proxy< render::Shader > m_shader;
	Ref< render::VertexBuffer > m_vertexBuffer;
	SmallMap< uint32_t, Glyph > m_glyphs;
};

	}
}

#endif	// traktor_spark_Font_H
