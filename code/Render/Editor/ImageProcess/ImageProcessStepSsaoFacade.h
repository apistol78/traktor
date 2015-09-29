#ifndef traktor_render_ImageProcessStepSsaoFacade_H
#define traktor_render_ImageProcessStepSsaoFacade_H

#include "Render/Editor/ImageProcess/IImageProcessStepFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessStepSsaoFacade : public IImageProcessStepFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessStep* step) const;

	virtual std::wstring getText(const ImageProcessStep* step) const;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const;

	virtual bool canHaveChildren() const;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const;
};

	}
}

#endif	// traktor_render_ImageProcessStepSsaoFacade_H
