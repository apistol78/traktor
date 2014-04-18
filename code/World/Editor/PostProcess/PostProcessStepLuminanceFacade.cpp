#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/Editor/PostProcess/PostProcessStepLuminanceFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepLuminanceFacade", 0, PostProcessStepLuminanceFacade, IPostProcessStepFacade)

int32_t PostProcessStepLuminanceFacade::getImage(const PostProcessStep* step) const
{
	return 2;
}

std::wstring PostProcessStepLuminanceFacade::getText(const PostProcessStep* step) const
{
	return L"Luminance";
}

void PostProcessStepLuminanceFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepLuminance* luminance = checked_type_cast< const PostProcessStepLuminance*, false >(step);
	outSources.push_back(luminance->getSource());
}

bool PostProcessStepLuminanceFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepLuminanceFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepLuminanceFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepLuminanceFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
