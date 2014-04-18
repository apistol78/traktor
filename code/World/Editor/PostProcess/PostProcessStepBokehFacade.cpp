#include "World/PostProcess/PostProcessStepBokeh.h"
#include "World/Editor/PostProcess/PostProcessStepBokehFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBokehFacade", 0, PostProcessStepBokehFacade, IPostProcessStepFacade)

int32_t PostProcessStepBokehFacade::getImage(const PostProcessStep* step) const
{
	return 2;
}

std::wstring PostProcessStepBokehFacade::getText(const PostProcessStep* step) const
{
	return L"Bokeh";
}

void PostProcessStepBokehFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepBokeh* bokeh = checked_type_cast< const PostProcessStepBokeh*, false >(step);
	for (std::vector< PostProcessStepBokeh::Source >::const_iterator i = bokeh->getSources().begin(); i != bokeh->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool PostProcessStepBokehFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepBokehFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBokehFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBokehFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
