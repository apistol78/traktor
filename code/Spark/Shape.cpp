/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Matrix33.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "Spark/ColorTransform.h"
#include "Spark/Shape.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

render::handle_t s_handleTextured = 0;
render::handle_t s_handleCurved = 0;
render::handle_t s_handleTransform = 0;
render::handle_t s_handleFillColor = 0;
render::handle_t s_handleTexture = 0;
render::handle_t s_handleCurveSign = 0;
render::handle_t s_handleColorTransform_Mul = 0;
render::handle_t s_handleColorTransform_Add = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Shape", Shape, Object)

Shape::Shape(
	render::Mesh* mesh,
	const resource::Proxy< render::Shader >& shader,
	const AlignedVector< Part >& parts,
	const Aabb2& bounds
)
:	m_mesh(mesh)
,	m_shader(shader)
,	m_parts(parts)
,	m_bounds(bounds)
{
	s_handleTextured = render::getParameterHandle(L"Spark_Textured");
	s_handleCurved = render::getParameterHandle(L"Spark_Curved");
	s_handleTransform = render::getParameterHandle(L"Spark_Transform");
	s_handleFillColor = render::getParameterHandle(L"Spark_FillColor");
	s_handleTexture = render::getParameterHandle(L"Spark_Texture");
	s_handleCurveSign = render::getParameterHandle(L"Spark_CurveSign");
	s_handleColorTransform_Mul = render::getParameterHandle(L"Spark_ColorTransform_Mul");
	s_handleColorTransform_Add = render::getParameterHandle(L"Spark_ColorTransform_Add");
}

void Shape::render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const
{
	const Matrix44 T(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	const std::vector< render::Mesh::Part >& parts = m_mesh->getParts();
	T_FATAL_ASSERT (parts.size() == m_parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		m_shader->setCombination(s_handleTextured, m_parts[i].texture.getResource() != 0);
		m_shader->setCombination(s_handleCurved, m_parts[i].curveSign != 0);

		if (!m_shader->getCurrentProgram())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Shape");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = parts[i].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(s_handleTransform, T);
		renderBlock->programParams->setFloatParameter(s_handleCurveSign, float(m_parts[i].curveSign));
		renderBlock->programParams->setTextureParameter(s_handleTexture, m_parts[i].texture);
		renderBlock->programParams->setVectorParameter(s_handleFillColor, m_parts[i].fillColor);
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
