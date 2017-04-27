/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/ImageProcess/ImageProcessStepSetTarget.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSetTargetFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSetTargetFacade", 0, ImageProcessStepSetTargetFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSetTargetFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 4;
}

std::wstring ImageProcessStepSetTargetFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	const ImageProcessStepSetTarget* targetStep = mandatory_non_null_type_cast< const ImageProcessStepSetTarget* >(step);
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
