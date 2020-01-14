#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image/Steps/ImageProcessStepChain.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepChain", 0, ImageProcessStepChain, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepChain::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	RefArray< Instance > instances;
	for (auto step : m_steps)
	{
		Ref< ImageProcessStep::Instance > instance = step->create(resourceManager, renderSystem, width, height);
		if (instance)
			instances.push_back(instance);
		else
			return nullptr;
	}
	return new InstanceChain(instances);
}

void ImageProcessStepChain::serialize(ISerializer& s)
{
	s >> MemberRefArray< ImageProcessStep >(L"steps", m_steps);
}

// Instance

ImageProcessStepChain::InstanceChain::InstanceChain(const RefArray< Instance >& instances)
:	m_instances(instances)
{
}

void ImageProcessStepChain::InstanceChain::destroy()
{
	for (auto instance : m_instances)
		instance->destroy();
	m_instances.clear();
}

void ImageProcessStepChain::InstanceChain::render(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	for (auto instance : m_instances)
	{
		instance->render(
			imageProcess,
			renderContext,
			sharedParams,
			params
		);
	}
}

	}
}
