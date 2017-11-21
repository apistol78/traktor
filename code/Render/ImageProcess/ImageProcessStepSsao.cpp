/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessStepSsao.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSsao", 1, ImageProcessStepSsao, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepSsao::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceSsao::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	AutoArrayPtr< uint8_t > data(new uint8_t [256 * 256 * 4]);
	SimpleTextureCreateDesc desc;
	Vector4 offsets[64];
	Vector4 directions[8];
	RandomGeometry random;

	for (int i = 0; i < sizeof_array(offsets); ++i)
	{
		float r = random.nextFloat() * (1.0f - 0.1f) + 0.1f;
		offsets[i] = random.nextUnit().xyz0() + Vector4(0.0f, 0.0f, 0.0f, r);
	}

	for (int i = 0; i < sizeof_array(directions); ++i)
	{
		float a =  TWO_PI * float(i) / sizeof_array(directions);
		float c = std::cos(a);
		float s = std::sin(a);
		directions[i] = Vector4(c, s, 0.0f, 0.0f);
	}

	for (uint32_t y = 0; y < 256; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
		{
			Vector4 normal = random.nextUnit();
			normal = normal * Scalar(0.5f) + Scalar(0.5f);

			data[(x + y * 256) * 4 + 0] = uint8_t(normal.x() * 255);
			data[(x + y * 256) * 4 + 1] = uint8_t(normal.y() * 255);
			data[(x + y * 256) * 4 + 2] = uint8_t(normal.z() * 255);
			data[(x + y * 256) * 4 + 3] = 0;
		}
	}

	desc.width = 256;
	desc.height = 256;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 256 * 4;
	desc.initialData[0].slicePitch = 0;

	Ref< ISimpleTexture > randomNormals = renderSystem->createSimpleTexture(desc);
	if (!randomNormals)
		return 0;

	for (uint32_t y = 0; y < 256; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
		{
			float a = random.nextFloat() * TWO_PI;
			float c = std::cos(a);
			float s = std::sin(a);
			float j = random.nextFloat();

			data[(x + y * 256) * 4 + 0] = uint8_t((c * 0.5f + 0.5f) * 255);
			data[(x + y * 256) * 4 + 1] = uint8_t((s * 0.5f + 0.5f) * 255);
			data[(x + y * 256) * 4 + 2] = uint8_t(j * 255);
			data[(x + y * 256) * 4 + 3] = 0;
		}
	}

	desc.width = 256;
	desc.height = 256;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 256 * 4;
	desc.initialData[0].slicePitch = 0;

	Ref< ISimpleTexture > randomRotations = renderSystem->createSimpleTexture(desc);
	if (!randomRotations)
		return 0;

	return new InstanceSsao(this, sources, offsets, directions, shader, randomNormals, randomRotations);
}

void ImageProcessStepSsao::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	
	if (s.getVersion< ImageProcessStepSsao >() >= 1)
		s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepSsao::Source::Source()
:	index(0)
{
}

void ImageProcessStepSsao::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

ImageProcessStepSsao::InstanceSsao::InstanceSsao(
	const ImageProcessStepSsao* step,
	const std::vector< Source >& sources,
	const Vector4 offsets[64],
	const Vector4 directions[8],
	const resource::Proxy< Shader >& shader,
	ISimpleTexture* randomNormals,
	ISimpleTexture* randomRotations
)
:	m_step(step)
,	m_sources(sources)
,	m_shader(shader)
,	m_randomNormals(randomNormals)
,	m_randomRotations(randomRotations)
,	m_handleViewEdgeTopLeft(getParameterHandle(L"ViewEdgeTopLeft"))
,	m_handleViewEdgeTopRight(getParameterHandle(L"ViewEdgeTopRight"))
,	m_handleViewEdgeBottomLeft(getParameterHandle(L"ViewEdgeBottomLeft"))
,	m_handleViewEdgeBottomRight(getParameterHandle(L"ViewEdgeBottomRight"))
,	m_handleProjection(getParameterHandle(L"Projection"))
,	m_handleOffsets(getParameterHandle(L"Offsets"))
,	m_handleDirections(getParameterHandle(L"Directions"))
,	m_handleRandomNormals(getParameterHandle(L"RandomNormals"))
,	m_handleRandomRotations(getParameterHandle(L"RandomRotations"))
,	m_handleMagicCoeffs(getParameterHandle(L"MagicCoeffs"))
{
	for (int i = 0; i < sizeof_array(m_offsets); ++i)
		m_offsets[i] = offsets[i];
	for (int i = 0; i < sizeof_array(m_directions); ++i)
		m_directions[i] = directions[i];
}

void ImageProcessStepSsao::InstanceSsao::destroy()
{
	m_randomNormals->destroy();
}

void ImageProcessStepSsao::InstanceSsao::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->prepareShader(m_shader);

	Scalar p11 = params.projection.get(0, 0);
	Scalar p22 = params.projection.get(1, 1);
	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6];

	m_shader->setVectorParameter(m_handleViewEdgeTopLeft, viewEdgeTopLeft);
	m_shader->setVectorParameter(m_handleViewEdgeTopRight, viewEdgeTopRight);
	m_shader->setVectorParameter(m_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	m_shader->setVectorParameter(m_handleViewEdgeBottomRight, viewEdgeBottomRight);
	m_shader->setMatrixParameter(m_handleProjection, params.projection);
	m_shader->setVectorArrayParameter(m_handleOffsets, m_offsets, sizeof_array(m_offsets));
	m_shader->setVectorArrayParameter(m_handleDirections, m_directions, sizeof_array(m_directions));
	m_shader->setTextureParameter(m_handleRandomNormals, m_randomNormals);
	m_shader->setTextureParameter(m_handleRandomRotations, m_randomRotations);
	m_shader->setVectorParameter(m_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		RenderTargetSet* source = imageProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	screenRenderer->draw(renderView, m_shader);
}

	}
}
