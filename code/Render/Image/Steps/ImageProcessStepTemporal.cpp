#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepTemporal.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepTemporal", 0, ImageProcessStepTemporal, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepTemporal::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	std::vector< InstanceTemporal::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceTemporal(this, shader, sources);
}

void ImageProcessStepTemporal::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepTemporal::Source::Source()
:	index(0)
{
}

void ImageProcessStepTemporal::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

ImageProcessStepTemporal::InstanceTemporal::InstanceTemporal(const ImageProcessStepTemporal* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
,	m_handleTime(getParameterHandle(L"Time"))
,	m_handleDeltaTime(getParameterHandle(L"DeltaTime"))
,	m_handleViewEdgeTopLeft(getParameterHandle(L"ViewEdgeTopLeft"))
,	m_handleViewEdgeTopRight(getParameterHandle(L"ViewEdgeTopRight"))
,	m_handleViewEdgeBottomLeft(getParameterHandle(L"ViewEdgeBottomLeft"))
,	m_handleViewEdgeBottomRight(getParameterHandle(L"ViewEdgeBottomRight"))
,	m_handleProjection(getParameterHandle(L"Projection"))
,	m_handleView(getParameterHandle(L"View"))
,	m_handleViewLast(getParameterHandle(L"ViewLast"))
,	m_handleViewInverse(getParameterHandle(L"ViewInverse"))
,	m_handleDeltaView(getParameterHandle(L"DeltaView"))
,	m_handleDeltaViewProj(getParameterHandle(L"DeltaViewProj"))
,	m_handleMagicCoeffs(getParameterHandle(L"MagicCoeffs"))
{
}

void ImageProcessStepTemporal::InstanceTemporal::destroy()
{
}

void ImageProcessStepTemporal::InstanceTemporal::render(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	Scalar p11 = params.projection.get(0, 0);
	Scalar p22 = params.projection.get(1, 1);
	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6];

	Matrix44 deltaView = params.lastView * params.view.inverse();

	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);
	pp->setFloatParameter(m_handleTime, m_time);
	pp->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	pp->setVectorParameter(m_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(m_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(m_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(m_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(m_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setMatrixParameter(m_handleProjection, params.projection);
	pp->setMatrixParameter(m_handleView, params.view);
	pp->setMatrixParameter(m_handleViewLast, params.lastView);
	pp->setMatrixParameter(m_handleViewInverse, params.view.inverse());
	pp->setMatrixParameter(m_handleDeltaView, deltaView);
	pp->setMatrixParameter(m_handleDeltaViewProj, params.projection * deltaView);
	for (const auto& s : m_sources)
	{
		ISimpleTexture* source = imageProcess->getTarget(s.source);
		if (source)
			pp->setTextureParameter(s.param, source);		
	}
	pp->endParameters(renderContext);

	imageProcess->getScreenRenderer()->draw(renderContext, m_shader, pp);

	m_time += params.deltaTime;
}

	}
}
