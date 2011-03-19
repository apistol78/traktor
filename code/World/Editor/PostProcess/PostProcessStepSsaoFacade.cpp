#include "World/Editor/PostProcess/PostProcessStepSsaoFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepSsaoFacade", PostProcessStepSsaoFacade, IPostProcessStepFacade)

int32_t PostProcessStepSsaoFacade::getImage(const PostProcessStep* step) const
{
	return 7;
}

std::wstring PostProcessStepSsaoFacade::getText(const PostProcessStep* step) const
{
	return L"Ambient occlusion";
}

bool PostProcessStepSsaoFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSsaoFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSsaoFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSsaoFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
