#include "Render/ImageProcess/ImageProcessDefineTarget.h"
#include "Render/Editor/ImageProcess/ImageProcessDefineTargetFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessDefineTargetFacade", ImageProcessDefineTargetFacade, IImageProcessDefineFacade)

int32_t ImageProcessDefineTargetFacade::getImage(const ImageProcessDefine* definition) const
{
	return 0;
}

std::wstring ImageProcessDefineTargetFacade::getText(const ImageProcessDefine* definition) const
{
	const ImageProcessDefineTarget* target = checked_type_cast< const ImageProcessDefineTarget* >(definition);
	return target->getId();
}

	}
}
