#include "World/Editor/PostProcess/PostProcessStepGodRayFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepGodRayFacade", PostProcessStepGodRayFacade, IPostProcessStepFacade)

int32_t PostProcessStepGodRayFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepGodRayFacade::getText(const PostProcessStep* step) const
{
	return L"GodRay";
}

bool PostProcessStepGodRayFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepGodRayFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepGodRayFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepGodRayFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
