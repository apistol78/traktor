/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepGrainFacade_H
#define traktor_render_ImageProcessStepGrainFacade_H

#include "Render/Editor/ImageProcess/IImageProcessStepFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessStepGrainFacade : public IImageProcessStepFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessStep* step) const T_OVERRIDE T_FINAL;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessStep* step) const T_OVERRIDE T_FINAL;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const T_OVERRIDE T_FINAL;

	virtual bool canHaveChildren() const T_OVERRIDE T_FINAL;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const T_OVERRIDE T_FINAL;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const T_OVERRIDE T_FINAL;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ImageProcessStepGrainFacade_H
