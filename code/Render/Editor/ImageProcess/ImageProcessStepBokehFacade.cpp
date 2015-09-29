#include "Render/ImageProcess/ImageProcessStepBokeh.h"
#include "Render/Editor/ImageProcess/ImageProcessStepBokehFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepBokehFacade", 0, ImageProcessStepBokehFacade, IImageProcessStepFacade)

int32_t ImageProcessStepBokehFacade::getImage(const ImageProcessStep* step) const
{
	return 2;
}

std::wstring ImageProcessStepBokehFacade::getText(const ImageProcessStep* step) const
{
	return L"Bokeh";
}

void ImageProcessStepBokehFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepBokeh* bokeh = checked_type_cast< const ImageProcessStepBokeh*, false >(step);
	for (std::vector< ImageProcessStepBokeh::Source >::const_iterator i = bokeh->getSources().begin(); i != bokeh->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepBokehFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepBokehFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepBokehFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepBokehFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
