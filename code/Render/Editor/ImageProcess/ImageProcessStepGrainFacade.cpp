#include "Render/ImageProcess/ImageProcessStepGrain.h"
#include "Render/Editor/ImageProcess/ImageProcessStepGrainFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepGrainFacade", 0, ImageProcessStepGrainFacade, IImageProcessStepFacade)

int32_t ImageProcessStepGrainFacade::getImage(const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepGrainFacade::getText(const ImageProcessStep* step) const
{
	return L"Grain";
}

void ImageProcessStepGrainFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepGrain* grain = checked_type_cast< const ImageProcessStepGrain*, false >(step);
	for (std::vector< ImageProcessStepGrain::Source >::const_iterator i = grain->getSources().begin(); i != grain->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepGrainFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepGrainFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepGrainFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepGrainFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
