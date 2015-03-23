#include "World/PostProcess/PostProcessDefineTexture.h"
#include "World/Editor/PostProcess/PostProcessDefineTextureFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessDefineTextureFacade", PostProcessDefineTextureFacade, IPostProcessDefineFacade)

int32_t PostProcessDefineTextureFacade::getImage(const PostProcessDefine* definition) const
{
	return 0;
}

std::wstring PostProcessDefineTextureFacade::getText(const PostProcessDefine* definition) const
{
	const PostProcessDefineTexture* texture = checked_type_cast< const PostProcessDefineTexture* >(definition);
	return L"-- Texture --";
}

	}
}
