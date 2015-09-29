#include "Render/ImageProcess/ImageProcessStepSetTarget.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSetTargetFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSetTargetFacade", 0, ImageProcessStepSetTargetFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSetTargetFacade::getImage(const ImageProcessStep* step) const
{
	return 4;
}

std::wstring ImageProcessStepSetTargetFacade::getText(const ImageProcessStep* step) const
{
	const ImageProcessStepSetTarget* targetStep = checked_type_cast< const ImageProcessStepSetTarget* >(step);
	return L"Set target \"" + targetStep->getTarget() + L"\"";
}

void ImageProcessStepSetTargetFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool ImageProcessStepSetTargetFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSetTargetFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSetTargetFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSetTargetFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
