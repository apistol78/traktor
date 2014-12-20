#include "World/PostProcess/PostProcessStepTemporal.h"
#include "World/Editor/PostProcess/PostProcessStepTemporalFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepTemporalFacade", 0, PostProcessStepTemporalFacade, IPostProcessStepFacade)

int32_t PostProcessStepTemporalFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepTemporalFacade::getText(const PostProcessStep* step) const
{
	return L"Temporal";
}

void PostProcessStepTemporalFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepTemporal* Temporal = checked_type_cast< const PostProcessStepTemporal*, false >(step);
	for (std::vector< PostProcessStepTemporal::Source >::const_iterator i = Temporal->getSources().begin(); i != Temporal->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool PostProcessStepTemporalFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepTemporalFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepTemporalFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepTemporalFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
