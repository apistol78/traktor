#include "World/PostProcess/PostProcessStepGodRay.h"
#include "World/Editor/PostProcess/PostProcessStepGodRayFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepGodRayFacade", 0, PostProcessStepGodRayFacade, IPostProcessStepFacade)

int32_t PostProcessStepGodRayFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepGodRayFacade::getText(const PostProcessStep* step) const
{
	return L"GodRay";
}

void PostProcessStepGodRayFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepGodRay* godRay = checked_type_cast< const PostProcessStepGodRay*, false >(step);
	for (std::vector< PostProcessStepGodRay::Source >::const_iterator i = godRay->getSources().begin(); i != godRay->getSources().end(); ++i)
		outSources.push_back(i->source);
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
