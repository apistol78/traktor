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

const render::handle_t c_techniques[] =
{
	render::getParameterHandle(L"Solid"),
	render::getParameterHandle(L"In"),
	render::getParameterHandle(L"Out")
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Shape", Shape, Object)

Shape::Shape(
	render::Mesh* mesh,
	const resource::Proxy< render::Shader >& shader,
	const std::vector< uint8_t >& parts,
	const Aabb2& bounds
)
:	m_mesh(mesh)
,	m_shader(shader)
,	m_parts(parts)
,	m_bounds(bounds)
{
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
		m_shader->setTechnique(c_techniques[m_parts[i]]);
		T_FATAL_ASSERT (m_shader->getCurrentProgram());

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Shape");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = parts[i].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(L"Spark_Transform", T);
		renderBlock->programParams->setVectorParameter(L"Spark_ColorTransform_Mul", Vector4(
			colorTransform.red[0],
			colorTransform.green[0],
			colorTransform.blue[0],
			colorTransform.alpha[0]
		));
		renderBlock->programParams->setVectorParameter(L"Spark_ColorTransform_Add", Vector4(
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
