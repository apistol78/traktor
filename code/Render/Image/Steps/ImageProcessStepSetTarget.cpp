#include "Core/Serialization/ISerializer.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepSetTarget.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSetTarget", 0, ImageProcessStepSetTarget, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepSetTarget::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	return new InstanceSetTarget(getParameterHandle(m_target));
}

void ImageProcessStepSetTarget::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"target", m_target);
}

// Instance

ImageProcessStepSetTarget::InstanceSetTarget::InstanceSetTarget(handle_t target)
:	m_target(target)
{
}

void ImageProcessStepSetTarget::InstanceSetTarget::destroy()
{
}

void ImageProcessStepSetTarget::InstanceSetTarget::render(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	imageProcess->setTarget(renderContext, m_target);
}

	}
}
