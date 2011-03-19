#include "World/PostProcess/PostProcessStepChain.h"
#include "World/Editor/PostProcess/PostProcessStepChainFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepChainFacade", PostProcessStepChainFacade, IPostProcessStepFacade)

int32_t PostProcessStepChainFacade::getImage(const PostProcessStep* step) const
{
	return 1;
}

std::wstring PostProcessStepChainFacade::getText(const PostProcessStep* step) const
{
	return L"Chain";
}

bool PostProcessStepChainFacade::canHaveChildren() const
{
	return true;
}

bool PostProcessStepChainFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepChainFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepChainFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	const PostProcessStepChain* chainStep = checked_type_cast< const PostProcessStepChain* >(step);
	outChildren = chainStep->getSteps();
	return true;
}

	}
}
