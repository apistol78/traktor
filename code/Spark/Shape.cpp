#include "Core/Math/Matrix33.h"
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

render::handle_t s_handleTransform = 0;
render::handle_t s_handleColorTransform_Mul = 0;
render::handle_t s_handleColorTransform_Add = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Shape", Shape, Object)

Shape::Shape(
	render::Mesh* mesh,
	const AlignedVector< Part >& parts,
	const Aabb2& bounds
)
:	m_mesh(mesh)
,	m_parts(parts)
,	m_bounds(bounds)
{
	s_handleTransform = render::getParameterHandle(L"Spark_Transform");
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
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Shape");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_parts[i].shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = parts[i].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(s_handleTransform, T);
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
