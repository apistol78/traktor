#include "World/Editor/PostProcess/PostProcessStepSmProjFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepSmProjFacade", PostProcessStepSmProjFacade, IPostProcessStepFacade)

int32_t PostProcessStepSmProjFacade::getImage(const PostProcessStep* step) const
{
	return 6;
}

std::wstring PostProcessStepSmProjFacade::getText(const PostProcessStep* step) const
{
	return L"Project shadow map";
}

bool PostProcessStepSmProjFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSmProjFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSmProjFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSmProjFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
