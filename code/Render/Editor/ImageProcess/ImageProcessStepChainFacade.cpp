/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/ImageProcess/ImageProcessStepChain.h"
#include "Render/Editor/ImageProcess/ImageProcessStepChainFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepChainFacade", 0, ImageProcessStepChainFacade, IImageProcessStepFacade)

int32_t ImageProcessStepChainFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 1;
}

std::wstring ImageProcessStepChainFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Chain";
}

void ImageProcessStepChainFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool ImageProcessStepChainFacade::canHaveChildren() const
{
	return true;
}

bool ImageProcessStepChainFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepChainFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepChainFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	const ImageProcessStepChain* chainStep = checked_type_cast< const ImageProcessStepChain* >(step);
	outChildren = chainStep->getSteps();
	return true;
}

	}
}
