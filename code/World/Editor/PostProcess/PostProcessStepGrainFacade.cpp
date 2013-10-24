#include "World/Editor/PostProcess/PostProcessStepGrainFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepGrainFacade", PostProcessStepGrainFacade, IPostProcessStepFacade)

int32_t PostProcessStepGrainFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepGrainFacade::getText(const PostProcessStep* step) const
{
	return L"Grain";
}

bool PostProcessStepGrainFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepGrainFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepGrainFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepGrainFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
