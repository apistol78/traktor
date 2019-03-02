#include "Render/ImageProcess/ImageProcessStepSsao.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSsaoFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSsaoFacade", 0, ImageProcessStepSsaoFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSsaoFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 7;
}

std::wstring ImageProcessStepSsaoFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Ambient occlusion";
}

void ImageProcessStepSsaoFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepSsao* ssao = mandatory_non_null_type_cast< const ImageProcessStepSsao* >(step);
	for (std::vector< ImageProcessStepSsao::Source >::const_iterator i = ssao->getSources().begin(); i != ssao->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepSsaoFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSsaoFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSsaoFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSsaoFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
