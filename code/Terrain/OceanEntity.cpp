#include <limits>
#include "Heightfield/Heightfield.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Terrain.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

struct WavesRenderBlock : public render::RenderBlock
{
	render::ScreenRenderer* screenRenderer;
	render::RenderTargetSet* targetWaves;

	virtual void render(render::IRenderView* renderView, const render::ProgramParameters* globalParameters) const
	{
		if (programParams)
			programParams->fixup(program);
		if (globalParameters)
			globalParameters->fixup(program);

		screenRenderer->draw(renderView, targetWaves, 0, program);
	}
};

struct OceanRenderBlock : public render::RenderBlock
{
	render::ScreenRenderer* screenRenderer;

	virtual void render(render::IRenderView* renderView, const render::ProgramParameters* globalParameters) const
	{
		if (programParams)
			programParams->fixup(program);
		if (globalParameters)
			globalParameters->fixup(program);

		screenRenderer->draw(renderView, program);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntity", OceanEntity, world::Entity)

OceanEntity::OceanEntity()
:	m_transform(Transform::identity())
{
}

bool OceanEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data)
{
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return 0;

	resourceManager->bind(data.m_terrain, m_terrain);

	if (!resourceManager->bind(data.m_shaderWaves, m_shaderWaves))
		return false;
	if (!resourceManager->bind(data.m_shaderComposite, m_shaderComposite))
		return false;

	for (int i = 0; i < MaxWaves; ++i)
		m_waveData[i] = Vector4(data.m_waves[i].direction.x, data.m_waves[i].direction.y, data.m_waves[i].amplitude, data.m_waves[i].phase);

	render::RenderTargetSetCreateDesc desc;
	desc.count = 1;
	desc.width = 1024;
	desc.height = 1024;
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.preferTiled = false;
	desc.ignoreStencil = true;
	desc.generateMips = false;
	desc.targets[0].format = render::TfR16F;
	
	m_targetWaves = renderSystem->createRenderTargetSet(desc);
	if (!m_targetWaves)
		return false;

	m_transform = data.getTransform();
	return true;
}

void OceanEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	const Frustum& viewFrustum = worldRenderView.getViewFrustum();
	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	Scalar p11 = projection.get(0, 0);
	Scalar p22 = projection.get(1, 1);
	Vector4 viewEdgeTopLeft = viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = viewFrustum.corners[6];

	worldRenderPass.setShaderTechnique(m_shaderComposite);
	worldRenderPass.setShaderCombination(m_shaderComposite);

	render::IProgram* program = m_shaderComposite->getCurrentProgram();
	if (!program)
		return;

	// Render wave displacement map.
	{
		WavesRenderBlock* renderBlock = renderContext->alloc< WavesRenderBlock >("Ocean waves");

		renderBlock->screenRenderer = m_screenRenderer;
		renderBlock->targetWaves = m_targetWaves;
		renderBlock->distance = 0.0f;
		renderBlock->program = m_shaderWaves->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			false
		);

		renderBlock->programParams->setVectorArrayParameter(L"WaveData", m_waveData, MaxWaves);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RfSetup, renderBlock);
	}

	// Render ocean compositing.
	{
		OceanRenderBlock* renderBlock = renderContext->alloc< OceanRenderBlock >("Ocean composite");

		renderBlock->screenRenderer = m_screenRenderer;
		renderBlock->distance = std::numeric_limits< float >::max();
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			false
		);

		renderBlock->programParams->setVectorParameter(L"ViewEdgeTopLeft", viewEdgeTopLeft);
		renderBlock->programParams->setVectorParameter(L"ViewEdgeTopRight", viewEdgeTopRight);
		renderBlock->programParams->setVectorParameter(L"ViewEdgeBottomLeft", viewEdgeBottomLeft);
		renderBlock->programParams->setVectorParameter(L"ViewEdgeBottomRight", viewEdgeBottomRight);
		renderBlock->programParams->setVectorParameter(L"MagicCoeffs", Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		renderBlock->programParams->setMatrixParameter(L"View", view);
		renderBlock->programParams->setMatrixParameter(L"ViewInverse", view.inverse());
		renderBlock->programParams->setMatrixParameter(L"Projection", projection);
		renderBlock->programParams->setVectorParameter(L"Eye", view.inverse().translation().xyz1());
		renderBlock->programParams->setFloatParameter(L"OceanAltitude", m_transform.translation().y());
		renderBlock->programParams->setTextureParameter(L"Waves", m_targetWaves->getColorTexture(0));

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RfAlphaBlend, renderBlock);
	}
}

void OceanEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool OceanEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 OceanEntity::getBoundingBox() const
{
	return Aabb3();
}

void OceanEntity::update(const UpdateParams& update)
{
}

	}
}
