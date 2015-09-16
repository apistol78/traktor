#include "Core/Math/Matrix33.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Spark/Font.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Font", Font, Object)

float Font::advance(uint32_t character) const
{
	return 1.0f;
}

void Font::render(render::RenderContext* renderContext, const Matrix33& transform, uint32_t character, float height) const
{
	const Matrix44 T(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	SmallMap< uint32_t, Vector4 >::const_iterator it = m_glyphRects.find(character);
	if (it != m_glyphRects.end())
	{
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Glyph");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitives.setNonIndexed(render::PtTriangles, 0, 2);

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(L"Spark_Transform", T);
		renderBlock->programParams->setTextureParameter(L"Spark_GlyphMap", m_texture);
		renderBlock->programParams->setVectorParameter(L"Spark_GlyphRect", it->second);
		renderBlock->programParams->setFloatParameter(L"Spark_GlyphHeight", height);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			render::RpOverlay,
			renderBlock
		);
	}
}

	}
}
