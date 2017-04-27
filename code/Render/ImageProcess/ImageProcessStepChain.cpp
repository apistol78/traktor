/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/ImageProcess/ImageProcessStepChain.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

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
	for (RefArray< ImageProcessStep >::const_iterator i = m_steps.begin(); i != m_steps.end(); ++i)
	{
		Ref< ImageProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem, width, height);
		if (instance)
			instances.push_back(instance);
		else
			return 0;
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
	for (RefArray< Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->destroy();
}

void ImageProcessStepChain::InstanceChain::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	for (RefArray< Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		(*i)->render(
			imageProcess,
			renderView,
			screenRenderer,
			params
		);
	}
}

	}
}
