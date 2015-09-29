#include "Render/ImageProcess/ImageProcessStepSwapTargets.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSwapTargetsFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSwapTargetsFacade", 0, ImageProcessStepSwapTargetsFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSwapTargetsFacade::getImage(const ImageProcessStep* step) const
{
	return 8;
}

std::wstring ImageProcessStepSwapTargetsFacade::getText(const ImageProcessStep* step) const
{
	const ImageProcessStepSwapTargets* swapStep = checked_type_cast< const ImageProcessStepSwapTargets* >(step);
	return L"Swap targets \"" + swapStep->getDestination() + L"\" <-> \"" + swapStep->getSource() + L"\"";
}

void ImageProcessStepSwapTargetsFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool ImageProcessStepSwapTargetsFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSwapTargetsFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSwapTargetsFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSwapTargetsFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
