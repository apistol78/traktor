#include "World/PostProcess/PostProcessStepSsao.h"
#include "World/Editor/PostProcess/PostProcessStepSsaoFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSsaoFacade", 0, PostProcessStepSsaoFacade, IPostProcessStepFacade)

int32_t PostProcessStepSsaoFacade::getImage(const PostProcessStep* step) const
{
	return 7;
}

std::wstring PostProcessStepSsaoFacade::getText(const PostProcessStep* step) const
{
	return L"Ambient occlusion";
}

void PostProcessStepSsaoFacade::getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const PostProcessStepSsao* ssao = checked_type_cast< const PostProcessStepSsao*, false >(step);
	for (std::vector< PostProcessStepSsao::Source >::const_iterator i = ssao->getSources().begin(); i != ssao->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool PostProcessStepSsaoFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSsaoFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSsaoFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSsaoFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
