/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/ImageProcess/ImageProcessStepLensDirt.h"
#include "Render/Editor/ImageProcess/ImageProcessStepLensDirtFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepLensDirtFacade", 0, ImageProcessStepLensDirtFacade, IImageProcessStepFacade)

int32_t ImageProcessStepLensDirtFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepLensDirtFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Lens Dirt";
}

void ImageProcessStepLensDirtFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepLensDirt* lensDirt = mandatory_non_null_type_cast< const ImageProcessStepLensDirt* >(step);
	for (std::vector< ImageProcessStepLensDirt::Source >::const_iterator i = lensDirt->getSources().begin(); i != lensDirt->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepLensDirtFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepLensDirtFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepLensDirtFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepLensDirtFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
