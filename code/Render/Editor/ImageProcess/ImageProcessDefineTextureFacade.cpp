#include "Render/ImageProcess/ImageProcessDefineTexture.h"
#include "Render/Editor/ImageProcess/ImageProcessDefineTextureFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessDefineTextureFacade", ImageProcessDefineTextureFacade, IImageProcessDefineFacade)

int32_t ImageProcessDefineTextureFacade::getImage(const ImageProcessDefine* definition) const
{
	return 0;
}

std::wstring ImageProcessDefineTextureFacade::getText(const ImageProcessDefine* definition) const
{
	const ImageProcessDefineTexture* texture = checked_type_cast< const ImageProcessDefineTexture* >(definition);
	return L"-- Texture --";
}

	}
}
