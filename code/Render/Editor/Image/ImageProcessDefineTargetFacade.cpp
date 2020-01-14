#include "Render/Image/Defines/ImageProcessDefineTarget.h"
#include "Render/Editor/Image/ImageProcessDefineTargetFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessDefineTargetFacade", ImageProcessDefineTargetFacade, IImageProcessDefineFacade)

int32_t ImageProcessDefineTargetFacade::getImage(editor::IEditor* editor, const ImageProcessDefine* definition) const
{
	return 0;
}

std::wstring ImageProcessDefineTargetFacade::getText(editor::IEditor* editor, const ImageProcessDefine* definition) const
{
	const ImageProcessDefineTarget* target = mandatory_non_null_type_cast< const ImageProcessDefineTarget* >(definition);
	return target->getId();
}

	}
}
