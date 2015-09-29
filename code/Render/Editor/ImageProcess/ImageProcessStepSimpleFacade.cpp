#include "Render/ImageProcess/ImageProcessStepSimple.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSimpleFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSimpleFacade", 0, ImageProcessStepSimpleFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSimpleFacade::getImage(const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepSimpleFacade::getText(const ImageProcessStep* step) const
{
	return L"Simple";
}

void ImageProcessStepSimpleFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepSimple* simple = checked_type_cast< const ImageProcessStepSimple*, false >(step);
	for (std::vector< ImageProcessStepSimple::Source >::const_iterator i = simple->getSources().begin(); i != simple->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepSimpleFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSimpleFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSimpleFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSimpleFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
