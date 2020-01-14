#include "Core/Serialization/ISerializer.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepLuminance.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepLuminance", 0, ImageProcessStepLuminance, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepLuminance::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

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
		getParameterHandle(m_source),
		sampleOffsets
	);
}

void ImageProcessStepLuminance::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> Member< std::wstring >(L"source", m_source);
}

// Instance

ImageProcessStepLuminance::InstanceLuminance::InstanceLuminance(
	const ImageProcessStepLuminance* step,
	const resource::Proxy< Shader >& shader,
	handle_t source,
	const Vector4 sampleOffsets[16]
)
:	m_step(step)
,	m_shader(shader)
,	m_source(source)
,	m_handleSourceTexture(getParameterHandle(L"SourceTexture"))
,	m_handleSampleOffsets(getParameterHandle(L"SampleOffsets"))
,	m_handleSampleOffsetScale(getParameterHandle(L"SampleOffsetScale"))
{
	for (int i = 0; i < sizeof_array(m_sampleOffsets); ++i)
		m_sampleOffsets[i] = sampleOffsets[i];
}

void ImageProcessStepLuminance::InstanceLuminance::destroy()
{
}

void ImageProcessStepLuminance::InstanceLuminance::build(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{

	ISimpleTexture* source = imageProcess->getTarget(m_source);
	if (!source)
		return;
	
	const Vector4 sampleOffsetScale(
		1.0f / source->getWidth(),
		1.0f / source->getHeight(),
		0.5f / source->getWidth(),
		0.5f / source->getHeight()
	);

	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);
	pp->setTextureParameter(L"SourceTexture", source);
	pp->setVectorArrayParameter(L"SampleOffsets", m_sampleOffsets, sizeof_array(m_sampleOffsets));
	pp->setVectorParameter(L"SampleOffsetScale", sampleOffsetScale);
	pp->endParameters(renderContext);

	imageProcess->getScreenRenderer()->draw(renderContext, m_shader, pp);
}

	}
}
