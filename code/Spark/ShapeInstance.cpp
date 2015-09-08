#pragma optimize( "", off )

#include "Render/Shader.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeInstance", ShapeInstance, CharacterInstance)

ShapeInstance::ShapeInstance(const Shape* shape)
:	m_shape(shape)
{
}

void ShapeInstance::render(render::RenderContext* renderContext) const
{
	render::IProgram* program = m_shape->m_shader->getCurrentProgram();
	if (!program)
		return;

	const std::vector< render::Mesh::Part >& parts = m_shape->m_mesh->getParts();
	for (size_t i = 0; i < parts.size(); ++i)
	{
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Shape");

		renderBlock->distance = 0.0f;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_shape->m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_shape->m_mesh->getVertexBuffer();
		renderBlock->primitives = parts[i].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setVectorParameter(L"Spark_Color", Vector4(
			float((std::rand() % 255) / 255.0f),
			float((std::rand() % 255) / 255.0f),
			float((std::rand() % 255) / 255.0f),
			1.0f
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
