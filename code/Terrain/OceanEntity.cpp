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

	if (!resourceManager->bind(data.m_shaderComposite, m_shaderComposite))
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
