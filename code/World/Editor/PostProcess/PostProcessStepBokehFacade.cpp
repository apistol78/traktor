#include "World/Editor/PostProcess/PostProcessStepBokehFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepBokehFacade", PostProcessStepBokehFacade, IPostProcessStepFacade)

int32_t PostProcessStepBokehFacade::getImage(const PostProcessStep* step) const
{
	return 2;
}

std::wstring PostProcessStepBokehFacade::getText(const PostProcessStep* step) const
{
	return L"Bokeh";
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
