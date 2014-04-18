#include "World/PostProcess/PostProcessStepLensDirt.h"
#include "World/Editor/PostProcess/PostProcessStepLensDirtFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepLensDirtFacade", 0, PostProcessStepLensDirtFacade, IPostProcessStepFacade)

int32_t PostProcessStepLensDirtFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepLensDirtFacade::getText(const PostProcessStep* step) const
{
	return L"Lens Dirt";
}

void PostProcessStepLensDirtFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepLensDirt* lensDirt = checked_type_cast< const PostProcessStepLensDirt*, false >(step);
	for (std::vector< PostProcessStepLensDirt::Source >::const_iterator i = lensDirt->getSources().begin(); i != lensDirt->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool PostProcessStepLensDirtFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepLensDirtFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepLensDirtFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepLensDirtFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
