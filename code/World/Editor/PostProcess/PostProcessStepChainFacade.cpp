#include "World/PostProcess/PostProcessStepChain.h"
#include "World/Editor/PostProcess/PostProcessStepChainFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepChainFacade", 0, PostProcessStepChainFacade, IPostProcessStepFacade)

int32_t PostProcessStepChainFacade::getImage(const PostProcessStep* step) const
{
	return 1;
}

std::wstring PostProcessStepChainFacade::getText(const PostProcessStep* step) const
{
	return L"Chain";
}

void PostProcessStepChainFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
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
