#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/ProbeRenderer.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< render::Shader > c_probeShader(Guid(L"{99BB18CB-A744-D845-9A17-D0E586E4D9EA}"));

#pragma pack(1)
struct Vertex
{
	float position[3];
};
#pragma pack()

render::handle_t s_handleProbeVolumeCenter;
render::handle_t s_handleProbeVolumeExtent;
render::handle_t s_handleProbeDiffuse;
render::handle_t s_handleProbeSpecular;
render::handle_t s_handleProbeSpecularMips;
render::handle_t s_handleMagicCoeffs;
render::handle_t s_handleWorldViewInv;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeRenderer", ProbeRenderer, IEntityRenderer)

ProbeRenderer::ProbeRenderer(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	resourceManager->bind(c_probeShader, m_probeShader);

	s_handleProbeVolumeCenter = render::getParameterHandle(L"World_ProbeVolumeCenter");
	s_handleProbeVolumeExtent = render::getParameterHandle(L"World_ProbeVolumeExtent");
	s_handleProbeDiffuse = render::getParameterHandle(L"World_ProbeDiffuse");
	s_handleProbeSpecular = render::getParameterHandle(L"World_ProbeSpecular");
	s_handleProbeSpecularMips = render::getParameterHandle(L"World_ProbeSpecularMips");
	s_handleMagicCoeffs = render::getParameterHandle(L"World_MagicCoeffs");
	s_handleWorldViewInv = render::getParameterHandle(L"World_WorldViewInv");

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 8 * sizeof(Vertex), false);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");

	Vector4 extents[8];
	Aabb3(Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)).getExtents(extents);

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	for (uint32_t i = 0; i < sizeof_array(extents); ++i)
	{
		vertex->position[0] = extents[i].x();
		vertex->position[1] = extents[i].y();
		vertex->position[2] = extents[i].z();
		vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, 6 * 2 * 3 * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	const int32_t* faces = Aabb3::getFaces();

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < 6; ++i)
	{
		*index++ = faces[i * 4 + 0];
		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 3];

		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 2];
		*index++ = faces[i * 4 + 3];
	}

	m_indexBuffer->unlock();
}

const TypeInfoSet ProbeRenderer::getRenderableTypes() const
{
	TypeInfoSet TypeInfoSet;
	TypeInfoSet.insert(&type_of< ProbeComponent >());
	return TypeInfoSet;
}

void ProbeRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ProbeComponent* probeComponent = checked_type_cast< ProbeComponent*, false >(renderable);
	const Transform& transform = probeComponent->getTransform();

	Matrix44 worldView = worldRenderView.getView() * transform.toMatrix44();

	Vector4 center = worldView * probeComponent->getVolume().getCenter().xyz1();
	Scalar radius = probeComponent->getVolume().getExtent().length();

	if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
		return;

	m_probeComponents.push_back(probeComponent);
}

void ProbeRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
	if (!m_probeShader)
		return;

	render::RenderContext* renderContext = worldContext.getRenderContext();
	T_ASSERT(renderContext);

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	for (auto probeComponent : m_probeComponents)
	{
		render::Shader* shader = m_probeShader;
		T_ASSERT(shader);

		worldRenderPass.setShaderTechnique(shader);
		worldRenderPass.setShaderCombination(shader);

		render::IProgram* program = shader->getCurrentProgram();
		if (!program)
			continue;

		const Transform& transform = probeComponent->getTransform();

		Matrix44 worldView = view * transform.toMatrix44();
		Matrix44 worldViewInv = worldView.inverse();

		render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("Probe");

		renderBlock->distance = 0.0f;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitive = render::PtTriangles;
		renderBlock->offset = 0;
		renderBlock->count = 12;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = 7;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			transform,
			transform,
			probeComponent->getBoundingBox()
		);

		renderBlock->programParams->setFloatParameter(s_handleProbeSpecularMips, probeComponent->getProbeSpecularTexture() != nullptr ? (float)probeComponent->getProbeSpecularTexture()->getMips() : 0.0f);
		renderBlock->programParams->setVectorParameter(s_handleProbeVolumeCenter, probeComponent->getVolume().getCenter());
		renderBlock->programParams->setVectorParameter(s_handleProbeVolumeExtent, probeComponent->getVolume().getExtent());
		renderBlock->programParams->setVectorParameter(s_handleMagicCoeffs, magicCoeffs);
		renderBlock->programParams->setMatrixParameter(s_handleWorldViewInv, worldViewInv);
		renderBlock->programParams->setTextureParameter(s_handleProbeDiffuse, probeComponent->getProbeDiffuseTexture());
		renderBlock->programParams->setTextureParameter(s_handleProbeSpecular, probeComponent->getProbeSpecularTexture());

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RpAlphaBlend, renderBlock);
	}

	// Flush all queued decals.
	m_probeComponents.resize(0);
}

	}
}
