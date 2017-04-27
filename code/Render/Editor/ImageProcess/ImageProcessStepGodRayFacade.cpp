/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/ImageProcess/ImageProcessStepGodRay.h"
#include "Render/Editor/ImageProcess/ImageProcessStepGodRayFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepGodRayFacade", 0, ImageProcessStepGodRayFacade, IImageProcessStepFacade)

int32_t ImageProcessStepGodRayFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepGodRayFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"GodRay";
}

void ImageProcessStepGodRayFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepGodRay* godRay = mandatory_non_null_type_cast< const ImageProcessStepGodRay* >(step);
	for (std::vector< ImageProcessStepGodRay::Source >::const_iterator i = godRay->getSources().begin(); i != godRay->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepGodRayFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepGodRayFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepGodRayFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepGodRayFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
