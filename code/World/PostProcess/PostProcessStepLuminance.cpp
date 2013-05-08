#include "Core/Serialization/ISerializer.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepLuminance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepLuminance", 0, PostProcessStepLuminance, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepLuminance::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	Vector4 sampleOffsets[16];
	int index = 0;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			sampleOffsets[index].set(
				float(x - 1.0f),
				float(y - 1.0f),
				0.0f,
				0.0f
			);
			index++;
		}
	}

	return new InstanceLuminance(
		this,
		shader,
		render::getParameterHandle(m_source),
		sampleOffsets
	);
}

void PostProcessStepLuminance::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< std::wstring >(L"source", m_source);
}

// Instance

PostProcessStepLuminance::InstanceLuminance::InstanceLuminance(
	const PostProcessStepLuminance* step,
	const resource::Proxy< render::Shader >& shader,
	render::handle_t source,
	const Vector4 sampleOffsets[16]
)
:	m_step(step)
,	m_shader(shader)
,	m_source(source)
,	m_handleSourceTexture(render::getParameterHandle(L"SourceTexture"))
,	m_handleSampleOffsets(render::getParameterHandle(L"SampleOffsets"))
,	m_handleSampleOffsetScale(render::getParameterHandle(L"SampleOffsetScale"))
{
	for (int i = 0; i < sizeof_array(m_sampleOffsets); ++i)
		m_sampleOffsets[i] = sampleOffsets[i];
}

void PostProcessStepLuminance::InstanceLuminance::destroy()
{
}

void PostProcessStepLuminance::InstanceLuminance::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	render::RenderTargetSet* source = postProcess->getTarget(m_source);
	if (!source)
		return;

	postProcess->prepareShader(m_shader);

	Vector4 sampleOffsetScale(
		1.0f / source->getWidth(),
		1.0f / source->getHeight(),
		0.5f / source->getWidth(),
		0.5f / source->getHeight()
	);

	m_shader->setTextureParameter(L"SourceTexture", source->getColorTexture(0));
	m_shader->setVectorArrayParameter(L"SampleOffsets", m_sampleOffsets, sizeof_array(m_sampleOffsets));
	m_shader->setVectorParameter(L"SampleOffsetScale", sampleOffsetScale);

	screenRenderer->draw(renderView, m_shader);
}

	}
}
