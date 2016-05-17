#ifndef traktor_render_ImageProcessStepSmProjFacade_H
#define traktor_render_ImageProcessStepSmProjFacade_H

#include "Render/Editor/ImageProcess/IImageProcessStepFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessStepSmProjFacade : public IImageProcessStepFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessStep* step) const T_OVERRIDE T_FINAL;

	virtual std::wstring getText(const ImageProcessStep* step) const T_OVERRIDE T_FINAL;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const T_OVERRIDE T_FINAL;

	virtual bool canHaveChildren() const T_OVERRIDE T_FINAL;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const T_OVERRIDE T_FINAL;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const T_OVERRIDE T_FINAL;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ImageProcessStepSmProjFacade_H
