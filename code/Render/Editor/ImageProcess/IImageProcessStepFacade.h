#ifndef traktor_render_IImageProcessStepFacade_H
#define traktor_render_IImageProcessStepFacade_H

#include <list>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace render
	{

class ImageProcessStep;

class IImageProcessStepFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessStep* step) const = 0;

	virtual std::wstring getText(const ImageProcessStep* step) const = 0;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const = 0;

	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const = 0;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const = 0;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const = 0;
};

	}
}

#endif	// traktor_render_IImageProcessStepFacade_H
