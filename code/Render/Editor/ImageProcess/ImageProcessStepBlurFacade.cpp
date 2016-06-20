#include "Render/ImageProcess/ImageProcessStepBlur.h"
#include "Render/Editor/ImageProcess/ImageProcessStepBlurFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepBlurFacade", 0, ImageProcessStepBlurFacade, IImageProcessStepFacade)

int32_t ImageProcessStepBlurFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 0;
}

std::wstring ImageProcessStepBlurFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Directional blur";
}

void ImageProcessStepBlurFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepBlur* blur = mandatory_non_null_type_cast< const ImageProcessStepBlur* >(step);
	for (std::vector< ImageProcessStepBlur::Source >::const_iterator i = blur->getSources().begin(); i != blur->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepBlurFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepBlurFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepBlurFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepBlurFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
