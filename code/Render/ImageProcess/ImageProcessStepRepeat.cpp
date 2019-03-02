#include "Render/ImageProcess/ImageProcessStepRepeat.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepRepeat", 0, ImageProcessStepRepeat, ImageProcessStep)

ImageProcessStepRepeat::ImageProcessStepRepeat()
:	m_count(0)
{
}

Ref< ImageProcessStep::Instance > ImageProcessStepRepeat::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	Ref< Instance > instance = m_step->create(resourceManager, renderSystem, width, height);
	if (!instance)
		return 0;

	return new InstanceRepeat(m_count, instance);
}

void ImageProcessStepRepeat::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< ImageProcessStep >(L"step", m_step);
}

// Instance

ImageProcessStepRepeat::InstanceRepeat::InstanceRepeat(uint32_t count, Instance* instance)
:	m_count(count)
,	m_instance(instance)
{
}

void ImageProcessStepRepeat::InstanceRepeat::destroy()
{
	m_instance->destroy();
}

void ImageProcessStepRepeat::InstanceRepeat::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	for (uint32_t i = 0; i < m_count; ++i)
	{
		m_instance->render(
			imageProcess,
			renderView,
			screenRenderer,
			params
		);
	}
}

	}
}
