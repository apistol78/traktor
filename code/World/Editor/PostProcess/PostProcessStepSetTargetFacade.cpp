#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/Editor/PostProcess/PostProcessStepSetTargetFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSetTargetFacade", 0, PostProcessStepSetTargetFacade, IPostProcessStepFacade)

int32_t PostProcessStepSetTargetFacade::getImage(const PostProcessStep* step) const
{
	return 4;
}

std::wstring PostProcessStepSetTargetFacade::getText(const PostProcessStep* step) const
{
	const PostProcessStepSetTarget* targetStep = checked_type_cast< const PostProcessStepSetTarget* >(step);
	return L"Set target \"" + targetStep->getTarget() + L"\"";
}

void PostProcessStepSetTargetFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool PostProcessStepSetTargetFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSetTargetFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSetTargetFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSetTargetFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
