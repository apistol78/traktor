#pragma once

#include "Render/Editor/ImageProcess/IImageProcessStepFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessStepSmProjFacade : public IImageProcessStepFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessStep* step) const override final;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessStep* step) const override final;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const override final;

	virtual bool canHaveChildren() const override final;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const override final;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const override final;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const override final;
};

	}
}

