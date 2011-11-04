#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

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
	if (!resourceManager->bind(m_shader))
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
		render::getParameterHandle(m_source),
		sampleOffsets
	);
}

bool PostProcessStepLuminance::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> Member< std::wstring >(L"source", m_source);
	return true;
}

// Instance

PostProcessStepLuminance::InstanceLuminance::InstanceLuminance(
	const PostProcessStepLuminance* step,
	render::handle_t source,
	const Vector4 sampleOffsets[16]
)
:	m_step(step)
,	m_source(source)
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
	resource::Proxy< render::Shader > shader = m_step->m_shader;
	if (!shader.validate())
		return;

	Ref< render::RenderTargetSet > source = postProcess->getTargetRef(m_source);
	if (!source)
		return;

	postProcess->prepareShader(shader);

	Vector4 sampleOffsetScale(
		1.0f / source->getWidth(),
		1.0f / source->getHeight(),
		0.5f / source->getWidth(),
		0.5f / source->getHeight()
	);

	shader->setTextureParameter(L"SourceTexture", source->getColorTexture(0));
	shader->setVectorArrayParameter(L"SampleOffsets", m_sampleOffsets, sizeof_array(m_sampleOffsets));
	shader->setVectorParameter(L"SampleOffsetScale", sampleOffsetScale);

	screenRenderer->draw(renderView, shader);
}

	}
}
