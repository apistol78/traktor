#include "Render/Shader.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
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

Shape::Shape(render::Mesh* mesh, const resource::Proxy< render::Shader >& shader, const std::vector< uint8_t >& parts)
:	m_mesh(mesh)
,	m_shader(shader)
,	m_parts(parts)
{
}

void Shape::render(render::RenderContext* renderContext, const Matrix33& transform) const
{
	const std::vector< render::Mesh::Part >& parts = m_mesh->getParts();
	T_ASSERT (parts.size() == m_parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		m_shader->setTechnique(c_techniques[m_parts[i]]);

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Shape");

		renderBlock->distance = 0.0f;
		renderBlock->program = m_shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = parts[i].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setMatrixParameter(L"Spark_Transform", Matrix44::identity());
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			render::RpOverlay,
			renderBlock
		);
	}
}

	}
}
