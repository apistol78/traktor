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
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepSsao.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSsao", 1, PostProcessStepSsao, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSsao::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_shader))
		return false;

	RandomGeometry random;
	Vector4 offsets[32];

	for (int i = 0; i < sizeof_array(offsets); ++i)
	{
		float r = random.nextFloat() * (1.0f - 0.1f) + 0.1f;
		offsets[i] = random.nextUnit() * Scalar(r);
	}

	AutoArrayPtr< uint8_t > data(new uint8_t [128 * 128 * 4]);
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			Vector4 normal = random.nextUnit() * Scalar(0.5f) + Scalar(0.5f);
			data[(x + y * 128) * 4 + 0] = uint8_t(normal.x() * 255);
			data[(x + y * 128) * 4 + 1] = uint8_t(normal.y() * 255);
			data[(x + y * 128) * 4 + 2] = uint8_t(normal.z() * 255);
			data[(x + y * 128) * 4 + 3] = 0;
		}
	}

	render::SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	Ref< render::ISimpleTexture > randomNormals = renderSystem->createSimpleTexture(desc);
	if (!randomNormals)
		return 0;

	return new InstanceSsao(this, offsets, randomNormals);
}

bool PostProcessStepSsao::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	
	if (s.getVersion() >= 1)
		s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);

	return true;
}

PostProcessStepSsao::Source::Source()
:	source(0)
,	index(0)
{
}

bool PostProcessStepSsao::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< int32_t >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepSsao::InstanceSsao::InstanceSsao(
	const PostProcessStepSsao* step,
	const Vector4 offsets[32],
	render::ISimpleTexture* randomNormals
)
:	m_step(step)
,	m_randomNormals(randomNormals)
{
	for (int i = 0; i < sizeof_array(m_offsets); ++i)
		m_offsets[i] = offsets[i];
}

void PostProcessStepSsao::InstanceSsao::destroy()
{
	m_randomNormals->destroy();
}

void PostProcessStepSsao::InstanceSsao::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	resource::Proxy< render::Shader > shader = m_step->m_shader;
	if (!shader.validate())
		return;

	Ref< render::RenderTargetSet > sourceColor = postProcess->getTargetRef(-1);
	Ref< render::RenderTargetSet > sourceDepth = postProcess->getTargetRef(-2);
	if (!sourceColor || !sourceDepth)
		return;

	postProcess->prepareShader(shader);

	Vector4 sourceColorSize(
		float(sourceColor->getWidth()),
		float(sourceColor->getHeight()),
		0.0f,
		0.0f
	);
	Vector4 sourceDepthSize(
		float(sourceDepth->getWidth()),
		float(sourceDepth->getHeight()),
		0.0f,
		0.0f
	);

	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6];

	shader->setVectorParameter(L"ViewEdgeTopLeft", viewEdgeTopLeft);
	shader->setVectorParameter(L"ViewEdgeTopRight", viewEdgeTopRight);
	shader->setVectorParameter(L"ViewEdgeBottomLeft", viewEdgeBottomLeft);
	shader->setVectorParameter(L"ViewEdgeBottomRight", viewEdgeBottomRight);
	shader->setMatrixParameter(L"Projection", params.projection);
	shader->setVectorArrayParameter(L"Offsets", m_offsets, sizeof_array(m_offsets));
	shader->setTextureParameter(L"RandomNormals", m_randomNormals);
	shader->setFloatParameter(L"DepthRange", params.depthRange);

	const std::vector< Source >& sources = m_step->m_sources;
	for (std::vector< Source >::const_iterator i = sources.begin(); i != sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			shader->setTextureParameter(i->param, source->getColorTexture(i->index));
			shader->setVectorParameter(i->param + L"_Size", Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	screenRenderer->draw(renderView, shader);
}

	}
}
