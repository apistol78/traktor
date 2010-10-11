#include "Core/Log/Log.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Partition/IPartition.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleUserParameter = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMesh", PartitionMesh, IMesh)

PartitionMesh::PartitionMesh()
{
	if (!s_handleUserParameter)
		s_handleUserParameter = render::getParameterHandle(L"UserParameter");
}

const Aabb& PartitionMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void PartitionMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Transform& worldTransform,
	const Transform& worldTransformPrevious,
	float distance,
	float userParameter,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_shader.validate() || !m_partition)
		return;

	std::set< uint32_t > partIndices;
	m_partition->traverse(
		worldRenderView->getCullFrustum(),
		worldTransform.toMatrix44(),
		worldRenderView->getView(),
		worldRenderView->getTechnique(),
		partIndices
	);
	if (partIndices.empty())
		return;

	Matrix44 worldView = worldRenderView->getView() * worldTransform.toMatrix44();
	uint32_t primitiveCount = 0;

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (std::set< uint32_t >::const_iterator i = partIndices.begin(); i != partIndices.end(); ++i)
	{
		const Part& part = m_parts[*i];

		m_shader->setTechnique(part.shaderTechnique);

		worldRenderView->setShaderCombination(
			m_shader,
			worldTransform.toMatrix44(),
			worldTransformPrevious.toMatrix44(),
			getBoundingBox()
		);

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

		Vector4 center = worldView * part.boundingBox.getCenter().xyz1();
		Scalar distancePart = center.length() + part.boundingBox.getExtent().length();

#if !defined(_DEBUG)
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("PartitionMesh");
#else
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(m_name.c_str());
#endif
		
		renderBlock->distance = distancePart;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = &meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		m_shader->setProgramParameters(renderBlock->programParams);
		worldRenderView->setProgramParameters(
			renderBlock->programParams,
			worldTransform.toMatrix44(),
			worldTransformPrevious.toMatrix44(),
			getBoundingBox()
		);
		renderBlock->programParams->setFloatParameter(s_handleUserParameter, userParameter);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			part.opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);

		primitiveCount += renderBlock->primitives->count;
	}

#if defined(_DEBUG)
	if (worldRenderView->getTechnique() == world::WorldRenderer::getTechniqueDefault())
		log::debug << L"PartitionMesh; " << primitiveCount << L" primitive(s) in " << uint32_t(partIndices.size()) << L" part(s)" << Endl;
#endif
}

	}
}
