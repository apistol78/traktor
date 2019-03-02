#include "Render/ImageProcess/ImageProcessStepLuminance.h"
#include "Render/Editor/ImageProcess/ImageProcessStepLuminanceFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepLuminanceFacade", 0, ImageProcessStepLuminanceFacade, IImageProcessStepFacade)

int32_t ImageProcessStepLuminanceFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 2;
}

std::wstring ImageProcessStepLuminanceFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Luminance";
}

void ImageProcessStepLuminanceFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepLuminance* luminance = mandatory_non_null_type_cast< const ImageProcessStepLuminance* >(step);
	outSources.push_back(luminance->getSource());
}

bool ImageProcessStepLuminanceFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepLuminanceFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepLuminanceFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepLuminanceFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
