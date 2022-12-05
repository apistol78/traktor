/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Float.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/IWorldRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldGatherContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/ProbeRenderer.h"

namespace traktor::world
{
	namespace
	{

#if !defined(__ANDROID__) && !defined(__IOS__)
const int32_t c_faceSize = 1024;
#else
const int32_t c_faceSize = 256;
#endif

const resource::Id< render::Shader > c_probeShader(Guid(L"{99BB18CB-A744-D845-9A17-D0E586E4D9EA}"));
const resource::Id< render::Shader > c_idFilterShader(Guid(L"{D9CC2267-0BDF-4A19-A970-856112821734}"));

const render::Handle c_handleProbeRoughness(L"World_ProbeRoughness");
const render::Handle c_handleProbeTexture(L"World_ProbeTexture");
const render::Handle c_handleProbeFilterCorners(L"World_ProbeFilterCorners");

#pragma pack(1)
struct Vertex
{
	float position[3];
};
#pragma pack()

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
,	m_captureState(0)
{
	resourceManager->bind(c_probeShader, m_probeShader);
	resourceManager->bind(c_idFilterShader, m_filterShader);

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, 4 + 8, sizeof(Vertex), false);
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

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, 2 * 3 + 6 * 2 * 3, sizeof(uint16_t), false);
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

	// Create screen renderer used for filtering each cube surface.
    m_screenRenderer = new render::ScreenRenderer();
    m_screenRenderer->create(m_renderSystem);

	// Create depth/stencil target which is used as "primary" depth
	// for world renderer since otherwise the world renderer
	// uses the frame's depth buffer.
	render::RenderTargetSetCreateDesc rtscd = {};
	rtscd.count = 0;
	rtscd.width = c_faceSize;
	rtscd.height = c_faceSize;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.usingDepthStencilAsTexture = false;
	rtscd.ignoreStencil = true;
	rtscd.generateMips = false;
	m_depthTargetSet = m_renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);
}

ProbeRenderer::~ProbeRenderer()
{
	safeDestroy(m_worldRenderer);
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
	safeDestroy(m_screenRenderer);
	safeDestroy(m_depthTargetSet);
	m_captureQueue.clear();
	m_capture = nullptr;
}

const TypeInfoSet ProbeRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ProbeComponent >();
}

void ProbeRenderer::gather(
	const WorldGatherContext& context,
	Object* renderable
)
{
	context.include(this, renderable);
}

void ProbeRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void ProbeRenderer::setup(const WorldSetupContext& context)
{
	render::RenderGraph& renderGraph = context.getRenderGraph();

	if (!m_capture)
	{
		// Get probe which needs to be updated.
		if (m_captureQueue.empty())
			return;
		m_capture = m_captureQueue.front();
		m_captureQueue.pop_front();
		m_captureState = 0;
		m_captureMip = 1;
	}
	T_ASSERT(m_capture);

	// Ensure probe hasn't been destroyed; could happen since
	// we ammortize probe capturing over a couple of frames.
	if (!m_capture->hasOwner())
	{
		m_capture = nullptr;
		return;
	}

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
		wrs.viewFarZ = 10000.0f;
		wrs.exposureMode = world::WorldRenderSettings::EmFixed;
		wrs.exposure = 0.0f;

		world::WorldCreateDesc wcd;
		wcd.worldRenderSettings = &wrs;
		wcd.entityRenderers = probeEntityRenderers;
		wcd.quality.toneMap = world::Quality::Medium;
		wcd.quality.motionBlur = world::Quality::Disabled;
		wcd.quality.reflections = world::Quality::Disabled;
		wcd.quality.shadows = world::Quality::Disabled;
		wcd.quality.ambientOcclusion = world::Quality::Disabled;
		wcd.quality.antiAlias = world::Quality::Disabled;
		wcd.quality.imageProcess = world::Quality::Disabled;
		wcd.multiSample = 0;
		wcd.gamma = 1.0f;
		wcd.sharedDepthStencil = m_depthTargetSet;

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

	render::ITexture* probeTexture = m_capture->getTexture();
	if (!probeTexture)
		return;

	if (m_captureState < 6)
	{
		const Vector4 pivot = m_capture->getTransform().translation().xyz1();
		const int32_t face = m_captureState;

		// Render world into first mip of reflection cube.
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
		worldRenderView.setSnapshot(true);
		worldRenderView.setPerspective(
			c_faceSize,
			c_faceSize,
			1.0f,
			deg2rad(90.0f),
			0.01f,
			10000.0f
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
#if !defined(__ANDROID__)
		rgtsd.targets[0].colorFormat = render::TfR11G11B10F;
#else
		rgtsd.targets[0].colorFormat = render::TfR8G8B8A8;
#endif
		auto faceTargetSetId = renderGraph.addTransientTargetSet(L"Probe render intermediate", rgtsd);

		// Render world to intermediate target.
		m_worldRenderer->setup(
			worldRenderView,
			context.getRootEntity(),
			renderGraph,
			faceTargetSetId
		);

		// Copy intermediate target to cubemap side.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Probe transfer");
		rp->addInput(faceTargetSetId);
		rp->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				auto faceTargetSet = renderGraph.getTargetSet(faceTargetSetId);
				auto lrb = renderContext->alloc< render::LambdaRenderBlock >(L"Probe transfer RT -> cube");
				lrb->lambda = [=](render::IRenderView* renderView)
				{
					const auto sz = probeTexture->getSize();

					render::Region sr = {};
					sr.x = 0;
					sr.y = 0;
					sr.mip = 0;
					sr.width = sz.x;
					sr.height = sz.y;

					render::Region dr = {};
					dr.x = 0;
					dr.y = 0;
					dr.z = face;
					dr.mip = 0;

					renderView->copy(probeTexture, dr, faceTargetSet->getColorTexture(0), sr);
				};
				renderContext->enqueue(lrb);
			}
		);
		renderGraph.addPass(rp);

		++m_captureState;
	}
	else if (m_captureState < 12)
	{
		// Generate mips by roughness filtering.
		const int32_t mipCount = (int32_t)log2(c_faceSize) + 1;
		const int32_t side = m_captureState - 6;
		const int32_t mip = m_captureMip;

		// Create intermediate target.
		render::RenderGraphTargetSetDesc rgtsd;
		rgtsd.count = 1;
		rgtsd.width = c_faceSize >> mip;
		rgtsd.height = c_faceSize >> mip;
		rgtsd.createDepthStencil = false;
		rgtsd.usingPrimaryDepthStencil = false;
		rgtsd.targets[0].colorFormat = render::TfR11G11B10F;
		auto filteredTargetSetId = renderGraph.addTransientTargetSet(L"Probe filter intermediate", rgtsd);

		Ref< render::RenderPass > filterPass = new render::RenderPass(L"Probe filter");
		filterPass->setOutput(filteredTargetSetId, render::TfNone, render::TfColor);
		filterPass->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				// Each mip represent one step rougher surface.
				const float roughness = std::pow((float)mip / mipCount, 0.5f);

				Vector4 corners[4];
				switch (side)
				{
				case 0:
					corners[0].set( 1.0f,  1.0f,  1.0f, 0.0f);
					corners[1].set( 1.0f,  1.0f, -1.0f, 0.0f);
					corners[2].set( 1.0f, -1.0f,  1.0f, 0.0f);
					corners[3].set( 1.0f, -1.0f, -1.0f, 0.0f);
					break;

				case 1:
					corners[0].set(-1.0f,  1.0f, -1.0f, 0.0f);
					corners[1].set(-1.0f,  1.0f,  1.0f, 0.0f);
					corners[2].set(-1.0f, -1.0f, -1.0f, 0.0f);
					corners[3].set(-1.0f, -1.0f,  1.0f, 0.0f);
					break;

				case 2:
					corners[0].set(-1.0f,  1.0f, -1.0f, 0.0f);
					corners[1].set( 1.0f,  1.0f, -1.0f, 0.0f);
					corners[2].set(-1.0f,  1.0f,  1.0f, 0.0f);
					corners[3].set( 1.0f,  1.0f,  1.0f, 0.0f);
					break;

				case 3:
					corners[0].set(-1.0f, -1.0f,  1.0f, 0.0f);
					corners[1].set( 1.0f, -1.0f,  1.0f, 0.0f);
					corners[2].set(-1.0f, -1.0f, -1.0f, 0.0f);
					corners[3].set( 1.0f, -1.0f, -1.0f, 0.0f);
					break;

				case 4:
					corners[0].set(-1.0f,  1.0f,  1.0f, 0.0f);
					corners[1].set( 1.0f,  1.0f,  1.0f, 0.0f);
					corners[2].set(-1.0f, -1.0f,  1.0f, 0.0f);
					corners[3].set( 1.0f, -1.0f,  1.0f, 0.0f);
					break;

				case 5:
					corners[0].set( 1.0f,  1.0f, -1.0f, 0.0f);
					corners[1].set(-1.0f,  1.0f, -1.0f, 0.0f);
					corners[2].set( 1.0f, -1.0f, -1.0f, 0.0f);
					corners[3].set(-1.0f, -1.0f, -1.0f, 0.0f);
					break;
				}

				auto pp = renderContext->alloc< render::ProgramParameters >();
				pp->beginParameters(renderContext);
				pp->setFloatParameter(c_handleProbeRoughness, roughness);
				pp->setTextureParameter(c_handleProbeTexture, probeTexture);
				pp->setVectorArrayParameter(c_handleProbeFilterCorners, corners, sizeof_array(corners));
				pp->endParameters(renderContext);

				m_screenRenderer->draw(renderContext, m_filterShader, pp);
			}
		);
		renderGraph.addPass(filterPass);

		// Write back filtered targets into cube map mip level.
		Ref< render::RenderPass > copyPass = new render::RenderPass(L"Probe copy filtered");
		copyPass->addInput(filteredTargetSetId);
		copyPass->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				auto filteredTargetSet = renderGraph.getTargetSet(filteredTargetSetId);
				auto lrb = renderContext->alloc< render::LambdaRenderBlock >(L"Probe transfer filtered -> cube");
				lrb->lambda = [=](render::IRenderView* renderView)
				{
					const auto sz = probeTexture->getSize();

					render::Region sr = {};
					sr.x = 0;
					sr.y = 0;
					sr.mip = 0;
					sr.width = sz.x >> mip;
					sr.height = sz.y >> mip;

					render::Region dr = {};
					dr.x = 0;
					dr.y = 0;
					dr.z = side;
					dr.mip = mip;

					renderView->copy(probeTexture, dr, filteredTargetSet->getColorTexture(0), sr);
				};
				renderContext->enqueue(lrb);
			}
		);
		renderGraph.addPass(copyPass);

		if (++m_captureMip >= mipCount)
		{
			m_captureMip = 1;
			m_captureState++;
		}
	}
	else
	{
		m_capture = nullptr;
		m_captureState = 0;
	}
}

void ProbeRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto probeComponent = static_cast< ProbeComponent* >(renderable);

	if (!m_probeShader)
		return;

	// Do not render probe which is being captured.
	if (probeComponent == m_capture)
		return;

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

	// Add to capture queue if probe is "dirty".
	if (probeComponent->getDirty() && probeComponent->shouldCapture())
	{
		m_captureQueue.push_back(probeComponent);
		probeComponent->setDirty(false);
		return;
	}

	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();
	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	render::IProgram* program = worldRenderPass.getProgram(m_probeShader).program;
	if (!program)
		return;

	const Transform& transform = probeComponent->getTransform();

	Matrix44 worldView = view * transform.toMatrix44();
	Matrix44 worldViewInv = worldView.inverse();

	auto rb = renderContext->alloc< render::IndexedRenderBlock >(L"Probe");

	rb->distance = 0.0f;
	rb->program = program;
	rb->programParams = renderContext->alloc< render::ProgramParameters >();
	rb->indexBuffer = m_indexBuffer->getBufferView();
	rb->indexType = render::IndexType::UInt16;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitive = render::PrimitiveType::Triangles;
		
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

	worldRenderPass.setProgramParameters(rb->programParams);

	if (probeComponent->getLocal())
	{
		rb->programParams->setVectorParameter(s_handleProbeVolumeCenter, probeComponent->getVolume().getCenter());
		rb->programParams->setVectorParameter(s_handleProbeVolumeExtent, probeComponent->getVolume().getExtent());
	}

	rb->programParams->setFloatParameter(s_handleProbeIntensity, probeComponent->getIntensity());
	rb->programParams->setFloatParameter(s_handleProbeTextureMips, probeComponent->getTexture() != nullptr ? (float)probeComponent->getTexture()->getSize().mips : 0.0f);
	rb->programParams->setVectorParameter(s_handleMagicCoeffs, magicCoeffs);
	rb->programParams->setMatrixParameter(s_handleWorldViewInv, worldViewInv);
	rb->programParams->setTextureParameter(s_handleProbeTexture, probeComponent->getTexture());

	rb->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, rb);
}

void ProbeRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

}
