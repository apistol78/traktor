#include "Render/ICubeTexture.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/IWorldRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/ProbeRenderer.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

#if !defined(__ANDROID__)
const int32_t c_faceSize = 1024;
#else
const int32_t c_faceSize = 128;
#endif

const resource::Id< render::Shader > c_probeShader(Guid(L"{99BB18CB-A744-D845-9A17-D0E586E4D9EA}"));

#pragma pack(1)
struct Vertex
{
	float position[3];
};
#pragma pack()

render::Handle s_handleProbeLocal(L"World_ProbeLocal");
render::Handle s_handleProbeVolumeCenter(L"World_ProbeVolumeCenter");
render::Handle s_handleProbeVolumeExtent(L"World_ProbeVolumeExtent");
render::Handle s_handleProbeTexture(L"World_ProbeTexture");
render::Handle s_handleProbeTextureMips(L"World_ProbeTextureMips");
render::Handle s_handleProbeIntensity(L"World_ProbeIntensity");
render::Handle s_handleMagicCoeffs(L"World_MagicCoeffs");
render::Handle s_handleWorldViewInv(L"World_WorldViewInv");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeRenderer", ProbeRenderer, IEntityRenderer)

ProbeRenderer::ProbeRenderer(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TypeInfo& worldRendererType
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_worldRendererType(worldRendererType)
{
	resourceManager->bind(c_probeShader, m_probeShader);

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, (4 + 8) * sizeof(Vertex), false);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");

	Vector4 extents[8];
	Aabb3(Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)).getExtents(extents);

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	// Quad vertices.
	vertex[0].position[0] = -1.0f; vertex[0].position[1] =  1.0f; vertex[0].position[2] = 0.0f;
	vertex[1].position[0] =  1.0f; vertex[1].position[1] =  1.0f; vertex[1].position[2] = 0.0f;
	vertex[2].position[0] =  1.0f; vertex[2].position[1] = -1.0f; vertex[2].position[2] = 0.0f;
	vertex[3].position[0] = -1.0f; vertex[3].position[1] = -1.0f; vertex[3].position[2] = 0.0f;
	vertex += 4;

	// Unit cube vertices.
	for (uint32_t i = 0; i < sizeof_array(extents); ++i)
	{
		vertex->position[0] = extents[i].x();
		vertex->position[1] = extents[i].y();
		vertex->position[2] = extents[i].z();
		vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, (2 * 3 + 6 * 2 * 3) * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT(index);
	
	// Quad faces.
	*index++ = 0;
	*index++ = 3;
	*index++ = 1;
	*index++ = 2;
	*index++ = 1;
	*index++ = 3;

	// Unit cube faces.
	const int32_t* faces = Aabb3::getFaces();
	for (uint32_t i = 0; i < 6; ++i)
	{
		*index++ = faces[i * 4 + 0] + 4;
		*index++ = faces[i * 4 + 1] + 4;
		*index++ = faces[i * 4 + 3] + 4;
		*index++ = faces[i * 4 + 1] + 4;
		*index++ = faces[i * 4 + 2] + 4;
		*index++ = faces[i * 4 + 3] + 4;
	}

	m_indexBuffer->unlock();
}

const TypeInfoSet ProbeRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ProbeComponent >();
}

void ProbeRenderer::gather(
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
}

void ProbeRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ProbeComponent* probeComponent = mandatory_non_null_type_cast< ProbeComponent* >(renderable);

	if (!m_probeShader)
		return;

	// Add to capture queue if probe is "dirty".
	if (probeComponent->getDirty())
	{
		m_captureQueue.push_back(probeComponent);
		probeComponent->setDirty(false);
	}

	// Cull local probes to frustum.
	if (probeComponent->getLocal())
	{
		const Transform& transform = probeComponent->getTransform();

		Matrix44 worldView = worldRenderView.getView() * transform.toMatrix44();

		Vector4 center = worldView * probeComponent->getVolume().getCenter().xyz1();
		Scalar radius = probeComponent->getVolume().getExtent().length();

		if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
			return;
	}

	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	worldRenderPass.setShaderTechnique(m_probeShader);
	worldRenderPass.setShaderCombination(m_probeShader);

	m_probeShader->setCombination(s_handleProbeLocal, probeComponent->getLocal());

	render::IProgram* program = m_probeShader->getCurrentProgram();
	if (!program)
		return;

	const Transform& transform = probeComponent->getTransform();

	Matrix44 worldView = view * transform.toMatrix44();
	Matrix44 worldViewInv = worldView.inverse();

	auto rb = renderContext->alloc< render::IndexedRenderBlock >("Probe");

	rb->distance = 0.0f;
	rb->program = program;
	rb->programParams = renderContext->alloc< render::ProgramParameters >();
	rb->indexBuffer = m_indexBuffer;
	rb->vertexBuffer = m_vertexBuffer;
	rb->primitive = render::PtTriangles;
		
	if (!probeComponent->getLocal())
	{
		rb->offset = 0;
		rb->count = 2;
		rb->minIndex = 0;
		rb->maxIndex = 3;
	}
	else
	{
		rb->offset = 6;
		rb->count = 12;
		rb->minIndex = 4;
		rb->maxIndex = 11;
	}

	rb->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		rb->programParams,
		transform,
		transform,
		probeComponent->getBoundingBox()
	);

	if (probeComponent->getLocal())
	{
		rb->programParams->setVectorParameter(s_handleProbeVolumeCenter, probeComponent->getVolume().getCenter());
		rb->programParams->setVectorParameter(s_handleProbeVolumeExtent, probeComponent->getVolume().getExtent());
	}

	rb->programParams->setFloatParameter(s_handleProbeIntensity, probeComponent->getIntensity());
	rb->programParams->setFloatParameter(s_handleProbeTextureMips, probeComponent->getTexture() != nullptr ? (float)probeComponent->getTexture()->getMips() : 0.0f);
	rb->programParams->setVectorParameter(s_handleMagicCoeffs, magicCoeffs);
	rb->programParams->setMatrixParameter(s_handleWorldViewInv, worldViewInv);
	rb->programParams->setTextureParameter(s_handleProbeTexture, probeComponent->getTexture());

	rb->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, rb);
}

void ProbeRenderer::flush(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void ProbeRenderer::setup(const WorldSetupContext& context)
{
	render::RenderGraph& renderGraph = context.getRenderGraph();

	// Get dirty probe which needs to be updated.
	if (m_captureQueue.empty())
		return;

	Ref< ProbeComponent > capture = m_captureQueue.front();
	m_captureQueue.pop_front();
	T_ASSERT(capture);

	capture->setDirty(false);

	// Lazy create world renderer, need to access entity renderers.
	if (!m_worldRenderer)
	{
		// Create a clone of world renderer without support to render probes.
		// This prevents nasty cyclic references of entity renderers.
		Ref< WorldEntityRenderers > probeEntityRenderers = new WorldEntityRenderers();
		for (auto er : context.getEntityRenderers()->get())
		{
			const TypeInfoSet renderableTypes = er->getRenderableTypes();
			if (renderableTypes.find(&type_of< ProbeComponent >()) == renderableTypes.end())
				probeEntityRenderers->add(er);
		}

		m_worldRenderer = mandatory_non_null_type_cast< world::IWorldRenderer* >(m_worldRendererType.createInstance());

		world::WorldRenderSettings wrs;
		wrs.viewNearZ = 0.01f;
		wrs.viewFarZ = 12000.0f;
		wrs.linearLighting = true;
		wrs.exposureMode = world::WorldRenderSettings::EmFixed;
		wrs.exposure = 0.0f;
		wrs.fog = false;

		world::WorldCreateDesc wcd;
		wcd.worldRenderSettings = &wrs;
		wcd.entityRenderers = probeEntityRenderers;
		wcd.toneMapQuality = world::QuMedium;
		wcd.motionBlurQuality = world::QuDisabled;
		wcd.reflectionsQuality = world::QuDisabled;
		wcd.shadowsQuality = world::QuDisabled;
		wcd.ambientOcclusionQuality = world::QuDisabled;
		wcd.antiAliasQuality = world::QuDisabled;
		wcd.imageProcessQuality = world::QuDisabled;
		wcd.multiSample = 0;
		wcd.frameCount = 1;
		wcd.gamma = 1.0f;
		wcd.sharedDepthStencil = nullptr;

		if (!m_worldRenderer->create(
			m_resourceManager,
			m_renderSystem,
			wcd
		))
		{
			m_worldRenderer = nullptr;
			return;
		}
	}

	render::ICubeTexture* probeTexture = capture->getTexture();
	if (!probeTexture)
		return;

	Vector4 pivot = capture->getTransform().translation().xyz1();

	// Render world into cube faces.
	for (int32_t face = 0; face < 6; ++face)
	{
		Matrix44 view;
		switch (face)
		{
		case 0:	// +X
			view = rotateY(deg2rad(-90.0f));
			break;
		case 1:	// -X
			view = rotateY(deg2rad( 90.0f));
			break;
		case 2:	// +Y
			view = rotateX(deg2rad( 90.0f));
			break;
		case 3: // -Y
			view = rotateX(deg2rad(-90.0f));
			break;
		case 4:	// +Z
			view = Matrix44::identity();
			break;
		case 5:	// -Z
			view = rotateY(deg2rad(180.0f));
			break;
		}

		// Move to pivot point.
		view = view * translate(pivot).inverse();

		// Render entities.
		world::WorldRenderView worldRenderView;
		worldRenderView.setPerspective(
			c_faceSize,
			c_faceSize,
			1.0f,
			deg2rad(90.0f),
			0.01f,
			12000.0f
		);
		worldRenderView.setTimes(0.0f, 1.0f / 60.0f, 0.0f);
		worldRenderView.setView(view, view);

		// Create intermediate target.
		render::RenderGraphTargetSetDesc rgtsd;
		rgtsd.count = 1;
		rgtsd.width = c_faceSize;
		rgtsd.height = c_faceSize;
		rgtsd.createDepthStencil = true;
		rgtsd.usingPrimaryDepthStencil = false;
		rgtsd.targets[0].colorFormat = render::TfR11G11B10F;
		auto faceTargetSetId = renderGraph.addTargetSet(rgtsd);

		// Render world to intermediate target.
		m_worldRenderer->setup(worldRenderView, renderGraph, faceTargetSetId);

		// Copy intermediate target to cubemap side.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Probe transfer");
		rp->addInput(faceTargetSetId);
		rp->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				auto faceTargetSet = renderGraph.getTargetSet(faceTargetSetId);
				auto lrb = renderContext->alloc< render::LambdaRenderBlock >();
				lrb->lambda = [=](render::IRenderView* renderView)
				{
					renderView->copy(
						probeTexture,
						face,
						0,
						faceTargetSet->getColorTexture(0),
						0,
						0
					);
				};
				renderContext->enqueue(lrb);
			}
		);
		renderGraph.addPass(rp);
	}
}

	}
}
