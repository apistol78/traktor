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

	const std::vector< render::Mesh::Part >& parts = instance->mesh[0]->getParts();
	for (size_t i = 0; i < parts.size(); ++i)
	{
		std::map< std::wstring, Part >::iterator it = m_parts.find(parts[i].name);
		if (it == m_parts.end())
			continue;

		if (!it->second.material.validate())
			continue;
		if (!it->second.material->hasTechnique(worldRenderView->getTechnique()))
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

		renderBlock->distance = distance;
		renderBlock->shader = it->second.material;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = instance->mesh[0]->getIndexBuffer();
		renderBlock->vertexBuffer = instance->mesh[0]->getVertexBuffer();
		renderBlock->primitives = &parts[i].primitives;

		renderBlock->shaderParams->beginParameters(renderContext);
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
			it->second.opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);
	}
}

	}
}
