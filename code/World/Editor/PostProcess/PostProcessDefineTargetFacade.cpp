#include "World/PostProcess/PostProcessDefineTarget.h"
#include "World/Editor/PostProcess/PostProcessDefineTargetFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessDefineTargetFacade", PostProcessDefineTargetFacade, IPostProcessDefineFacade)

int32_t PostProcessDefineTargetFacade::getImage(const PostProcessDefine* definition) const
{
	return 0;
}

std::wstring PostProcessDefineTargetFacade::getText(const PostProcessDefine* definition) const
{
	const PostProcessDefineTarget* target = checked_type_cast< const PostProcessDefineTarget* >(definition);
	return target->getId();
}

	}
}
