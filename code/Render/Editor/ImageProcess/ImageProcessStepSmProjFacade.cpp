#include "Render/ImageProcess/ImageProcessStepSmProj.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSmProjFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSmProjFacade", 0, ImageProcessStepSmProjFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSmProjFacade::getImage(const ImageProcessStep* step) const
{
	return 6;
}

std::wstring ImageProcessStepSmProjFacade::getText(const ImageProcessStep* step) const
{
	return L"Project shadow map";
}

void ImageProcessStepSmProjFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool ImageProcessStepSmProjFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSmProjFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSmProjFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSmProjFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
