#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/Editor/PostProcess/PostProcessStepSimpleFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSimpleFacade", 0, PostProcessStepSimpleFacade, IPostProcessStepFacade)

int32_t PostProcessStepSimpleFacade::getImage(const PostProcessStep* step) const
{
	return 5;
}

std::wstring PostProcessStepSimpleFacade::getText(const PostProcessStep* step) const
{
	return L"Simple";
}

void PostProcessStepSimpleFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepSimple* simple = checked_type_cast< const PostProcessStepSimple*, false >(step);
	for (std::vector< PostProcessStepSimple::Source >::const_iterator i = simple->getSources().begin(); i != simple->getSources().end(); ++i)
		outSources.push_back(i->source);
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
