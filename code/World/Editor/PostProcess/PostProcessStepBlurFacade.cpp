#include "World/Editor/PostProcess/PostProcessStepBlurFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepBlurFacade", PostProcessStepBlurFacade, IPostProcessStepFacade)

int32_t PostProcessStepBlurFacade::getImage(const PostProcessStep* step) const
{
	return 0;
}

std::wstring PostProcessStepBlurFacade::getText(const PostProcessStep* step) const
{
	return L"Directional blur";
}

bool PostProcessStepBlurFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepBlurFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBlurFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBlurFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
