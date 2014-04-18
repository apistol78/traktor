#include "World/PostProcess/PostProcessStepGrain.h"
#include "World/Editor/PostProcess/PostProcessStepGrainFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepGrainFacade", 0, PostProcessStepGrainFacade, IPostProcessStepFacade)

int32_t PostProcessStepGrainFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepGrainFacade::getText(const PostProcessStep* step) const
{
	return L"Grain";
}

void PostProcessStepGrainFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepGrain* grain = checked_type_cast< const PostProcessStepGrain*, false >(step);
	for (std::vector< PostProcessStepGrain::Source >::const_iterator i = grain->getSources().begin(); i != grain->getSources().end(); ++i)
		outSources.push_back(i->source);
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
