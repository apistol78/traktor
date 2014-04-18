#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/Editor/PostProcess/PostProcessStepBlurFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBlurFacade", 0, PostProcessStepBlurFacade, IPostProcessStepFacade)

int32_t PostProcessStepBlurFacade::getImage(const PostProcessStep* step) const
{
	return 0;
}

std::wstring PostProcessStepBlurFacade::getText(const PostProcessStep* step) const
{
	return L"Directional blur";
}

void PostProcessStepBlurFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepBlur* blur = checked_type_cast< const PostProcessStepBlur*, false >(step);
	for (std::vector< PostProcessStepBlur::Source >::const_iterator i = blur->getSources().begin(); i != blur->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool PostProcessStepBlurFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepBlurFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBlurFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepBlurFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
