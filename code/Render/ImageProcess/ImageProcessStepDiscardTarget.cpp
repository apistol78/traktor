#include "Render/ImageProcess/ImageProcessStepDiscardTarget.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepDiscardTarget", 0, ImageProcessStepDiscardTarget, ImageProcessStep)

ImageProcessStepDiscardTarget::ImageProcessStepDiscardTarget()
{
}

ImageProcessStepDiscardTarget::ImageProcessStepDiscardTarget(const std::wstring& target)
:	m_target(target)
{
}

Ref< ImageProcessStep::Instance > ImageProcessStepDiscardTarget::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	return new InstanceDiscardTarget(getParameterHandle(m_target));
}

void ImageProcessStepDiscardTarget::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"target", m_target);
}

// Instance

ImageProcessStepDiscardTarget::InstanceDiscardTarget::InstanceDiscardTarget(handle_t target)
:	m_target(target)
{
}

void ImageProcessStepDiscardTarget::InstanceDiscardTarget::destroy()
{
}

void ImageProcessStepDiscardTarget::InstanceDiscardTarget::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->discardTarget(m_target);
}

	}
}
