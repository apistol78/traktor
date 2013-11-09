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
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

render::handle_t s_handleReflectionEnable;
render::handle_t s_handleViewEdgeTopLeft;
render::handle_t s_handleViewEdgeTopRight;
render::handle_t s_handleViewEdgeBottomLeft;
render::handle_t s_handleViewEdgeBottomRight;
render::handle_t s_handleMagicCoeffs;
render::handle_t s_handleView;
render::handle_t s_handleViewInverse;
render::handle_t s_handleProjection;
render::handle_t s_handleEye;
render::handle_t s_handleOceanAltitude;

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
	s_handleReflectionEnable = render::getParameterHandle(L"ReflectionEnable");
	s_handleViewEdgeTopLeft = render::getParameterHandle(L"ViewEdgeTopLeft");
	s_handleViewEdgeTopRight = render::getParameterHandle(L"ViewEdgeTopRight");
	s_handleViewEdgeBottomLeft = render::getParameterHandle(L"ViewEdgeBottomLeft");
	s_handleViewEdgeBottomRight = render::getParameterHandle(L"ViewEdgeBottomRight");
	s_handleMagicCoeffs = render::getParameterHandle(L"MagicCoeffs");
	s_handleView = render::getParameterHandle(L"View");
	s_handleViewInverse = render::getParameterHandle(L"ViewInverse");
	s_handleProjection = render::getParameterHandle(L"Projection");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleOceanAltitude = render::getParameterHandle(L"OceanAltitude");
}

bool OceanEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data)
{
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return 0;

	if (!resourceManager->bind(data.m_shaderComposite, m_shaderComposite))
		return false;

	m_transform = data.getTransform();
	return true;
}

void OceanEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	bool reflectionEnable
)
{
	const Frustum& viewFrustum = worldRenderView.getViewFrustum();
	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	Matrix44 viewInv = view.inverse();
	Vector4 eye = viewInv.translation().xyz1();
	if (eye.y() < m_transform.translation().y())
		return;

	Scalar p11 = projection.get(0, 0);
	Scalar p22 = projection.get(1, 1);
	Vector4 viewEdgeTopLeft = viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = viewFrustum.corners[6];

	worldRenderPass.setShaderTechnique(m_shaderComposite);
	worldRenderPass.setShaderCombination(m_shaderComposite);

	m_shaderComposite->setCombination(s_handleReflectionEnable, reflectionEnable);

	render::IProgram* program = m_shaderComposite->getCurrentProgram();
	if (!program)
		return;

	OceanRenderBlock* renderBlock = renderContext->alloc< OceanRenderBlock >("Ocean");

	renderBlock->screenRenderer = m_screenRenderer;
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		m_shaderComposite->getCurrentPriority()
	);

	renderBlock->programParams->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	renderBlock->programParams->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	renderBlock->programParams->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	renderBlock->programParams->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	renderBlock->programParams->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	renderBlock->programParams->setMatrixParameter(s_handleView, view);
	renderBlock->programParams->setMatrixParameter(s_handleViewInverse, viewInv);
	renderBlock->programParams->setMatrixParameter(s_handleProjection, projection);
	renderBlock->programParams->setVectorParameter(s_handleEye, eye);
	renderBlock->programParams->setFloatParameter(s_handleOceanAltitude, m_transform.translation().y());

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shaderComposite->getCurrentPriority(), renderBlock);
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

void OceanEntity::update(const world::UpdateParams& update)
{
}

	}
}
