#include <algorithm>
#include "Core/Io/IStream.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "World/WorldRenderView.h"

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

const Aabb& StreamMesh::getBoundingBox() const
{
	return m_boundingBox;
}

uint32_t StreamMesh::getFrameCount() const
{
	return m_frameOffsets.size();
}

Ref< StreamMesh::Instance > StreamMesh::createInstance() const
{
	Ref< Instance > instance = new Instance();
	instance->frame = ~0UL;
	return instance;
}

void StreamMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Transform& worldTransform,
	const Transform& worldTransformPrevious,
	Instance* instance,
	uint32_t frame,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_shader.validate())
		return;

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

	std::map< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderView->getTechnique());
	if (it == m_parts.end())
		return;

	const std::vector< render::Mesh::Part >& meshParts = instance->mesh[0]->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		// \fixme Linear search by string
		std::vector< render::Mesh::Part >::const_iterator j = std::find_if(meshParts.begin(), meshParts.end(), NamedMeshPart(i->meshPart));
		if (j == meshParts.end())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("StreamMesh");

		renderBlock->distance = distance;
		renderBlock->shader = m_shader;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = instance->mesh[0]->getIndexBuffer();
		renderBlock->vertexBuffer = instance->mesh[0]->getVertexBuffer();
		renderBlock->primitives = &j->primitives;

		renderBlock->shaderParams->beginParameters(renderContext);
		renderBlock->shaderParams->setTechnique(i->shaderTechnique);
		worldRenderView->setShaderParameters(
			renderBlock->shaderParams,
			worldTransform.toMatrix44(),
			worldTransformPrevious.toMatrix44(),
			getBoundingBox()
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->shaderParams);
		renderBlock->shaderParams->endParameters(renderContext);

		renderContext->draw(
			i->opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);
	}
}

	}
}
