#include "Core/Math/Matrix33.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Spark/ColorTransform.h"
#include "Spark/Font.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

render::handle_t s_handleTransform = render::getParameterHandle(L"Spark_Transform");
render::handle_t s_handleGlyphMap = render::getParameterHandle(L"Spark_GlyphMap");
render::handle_t s_handleGlyphRect = render::getParameterHandle(L"Spark_GlyphRect");
render::handle_t s_handleGlyphUnit = render::getParameterHandle(L"Spark_GlyphUnit");
render::handle_t s_handleGlyphOffset = render::getParameterHandle(L"Spark_GlyphOffset");
render::handle_t s_handleGlyphHeight = render::getParameterHandle(L"Spark_GlyphHeight");
render::handle_t s_handleColorTransform_Mul = render::getParameterHandle(L"Spark_ColorTransform_Mul");
render::handle_t s_handleColorTransform_Add = render::getParameterHandle(L"Spark_ColorTransform_Add");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Font", Font, Object)

float Font::advance(uint32_t character) const
{
	SmallMap< uint32_t, Glyph >::const_iterator it = m_glyphs.find(character);
	if (it != m_glyphs.end())
		return it->second.advance;
	else
		return 0.0f;
}

void Font::render(render::RenderContext* renderContext, const Matrix33& transform, uint32_t character, float height, const ColorTransform& colorTransform) const
{
	const Matrix44 T(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	SmallMap< uint32_t, Glyph >::const_iterator it = m_glyphs.find(character);
	if (it != m_glyphs.end())
	{
		T_FATAL_ASSERT (m_shader->getCurrentProgram());

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Glyph");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitives.setNonIndexed(render::PtTriangles, 0, 2);

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(s_handleTransform, T);
		renderBlock->programParams->setTextureParameter(s_handleGlyphMap, m_texture);
		renderBlock->programParams->setVectorParameter(s_handleGlyphRect, it->second.rect);
		renderBlock->programParams->setVectorParameter(s_handleGlyphUnit, it->second.unit);
		renderBlock->programParams->setVectorParameter(s_handleGlyphOffset, it->second.offset);
		renderBlock->programParams->setFloatParameter(s_handleGlyphHeight, height);
		renderBlock->programParams->setVectorParameter(s_handleColorTransform_Mul, Vector4(
			colorTransform.red[0],
			colorTransform.green[0],
			colorTransform.blue[0],
			colorTransform.alpha[0]
		));
		renderBlock->programParams->setVectorParameter(s_handleColorTransform_Add, Vector4(
			colorTransform.red[1],
			colorTransform.green[1],
			colorTransform.blue[1],
			colorTransform.alpha[1]
		));
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			render::RpOverlay,
			renderBlock
		);
	}
}

	}
}
