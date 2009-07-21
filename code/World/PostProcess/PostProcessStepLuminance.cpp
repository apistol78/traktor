#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepLuminance", PostProcessStepLuminance, PostProcessStep)

bool PostProcessStepLuminance::create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	if (!resourceManager->bind(m_shader))
		return false;

	int index = 0;
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			m_sampleOffsets[index].set(
				float(x - 1.0f),
				float(y - 1.0f),
				0.0f,
				0.0f
			);
			index++;
		}
	}

	return true;
}

void PostProcessStepLuminance::destroy(PostProcess* postProcess)
{
}

void PostProcessStepLuminance::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	if (!m_shader.validate())
		return;

	Ref< render::RenderTargetSet > source = postProcess->getTargetRef(m_source);
	if (!source)
		return;

	Vector4 sampleOffsetScale(
		1.0f / source->getWidth(),
		1.0f / source->getHeight(),
		0.5f / source->getWidth(),
		0.5f / source->getHeight()
	);

	m_shader->setSamplerTexture(L"SourceTexture", source->getColorTexture(0));
	m_shader->setVectorArrayParameter(L"SampleOffsets", m_sampleOffsets, sizeof_array(m_sampleOffsets));
	m_shader->setVectorParameter(L"SampleOffsetScale", sampleOffsetScale);

	screenRenderer->draw(renderView, m_shader);
}

bool PostProcessStepLuminance::serialize(Serializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> Member< uint32_t >(L"source", m_source);
	return true;
}

	}
}
