#include "World/Editor/PostProcess/PostProcessStepLuminanceFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepLuminanceFacade", PostProcessStepLuminanceFacade, IPostProcessStepFacade)

int32_t PostProcessStepLuminanceFacade::getImage(const PostProcessStep* step) const
{
	return 2;
}

std::wstring PostProcessStepLuminanceFacade::getText(const PostProcessStep* step) const
{
	return L"Luminance";
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
