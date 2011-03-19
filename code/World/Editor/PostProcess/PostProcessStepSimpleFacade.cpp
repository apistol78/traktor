#include "World/Editor/PostProcess/PostProcessStepSimpleFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepSimpleFacade", PostProcessStepSimpleFacade, IPostProcessStepFacade)

int32_t PostProcessStepSimpleFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepSimpleFacade::getText(const PostProcessStep* step) const
{
	return L"Simple";
}

bool PostProcessStepSimpleFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSimpleFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSimpleFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSimpleFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
