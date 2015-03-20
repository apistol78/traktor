#include <algorithm>
#include "Core/Io/IStream.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

struct NamedMeshPart
{
	std::wstring m_meshPart;

	NamedMeshPart(const std::wstring& meshPart)
	:	m_meshPart(meshPart)
	{
	}

	bool operator () (const render::Mesh::Part& meshPart) const
	{
		return meshPart.name == m_meshPart;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMesh", StreamMesh, IMesh)

StreamMesh::StreamMesh()
{
}

const Aabb3& StreamMesh::getBoundingBox() const
{
	return m_boundingBox;
}

bool StreamMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

uint32_t StreamMesh::getFrameCount() const
{
	return uint32_t(m_frameOffsets.size());
}

Ref< StreamMesh::Instance > StreamMesh::createInstance() const
{
	Ref< Instance > instance = new Instance();
	instance->frame = ~0U;
	return instance;
}

void StreamMesh::render(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	Instance* instance,
	uint32_t frame,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	// Load mesh frame if different from instance's cached frame.
	if (instance->frame != frame || !instance->mesh)
	{
		m_stream->seek(IStream::SeekSet, m_frameOffsets[frame]);
		instance->mesh[1] = instance->mesh[0];
		instance->mesh[0] = m_meshReader->read(m_stream);
		instance->frame = frame;
	}

	if (!instance->mesh[0])
		return;

	SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT (it != m_parts.end());

	const std::vector< render::Mesh::Part >& meshParts = instance->mesh[0]->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		m_shader->setTechnique(i->shaderTechnique);

		worldRenderPass.setShaderCombination(
			m_shader,
			worldTransform.toMatrix44(),
			getBoundingBox()
		);

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

		// \fixme Linear search by string
		std::vector< render::Mesh::Part >::const_iterator j = std::find_if(meshParts.begin(), meshParts.end(), NamedMeshPart(i->meshPart));
		if (j == meshParts.end())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("StreamMesh");

		renderBlock->distance = distance;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = instance->mesh[0]->getIndexBuffer();
		renderBlock->vertexBuffer = instance->mesh[0]->getVertexBuffer();
		renderBlock->primitives = j->primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			m_shader->getCurrentPriority(),
			worldTransform.toMatrix44(),
			getBoundingBox()
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			m_shader->getCurrentPriority(),
			renderBlock
		);
	}
}

	}
}
