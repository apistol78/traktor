#ifndef traktor_world_IPostProcessStepFacade_H
#define traktor_world_IPostProcessStepFacade_H

#include <list>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace world
	{

class PostProcessStep;

class IPostProcessStepFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const PostProcessStep* step) const = 0;

	virtual std::wstring getText(const PostProcessStep* step) const = 0;

	virtual void getSources(const PostProcessStep* step, std::vector< std::wstring >& outSources) const = 0;

	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const = 0;

	virtual bool removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const = 0;

	virtual bool getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const = 0;
};

	}
}

#endif	// traktor_world_IPostProcessStepFacade_H
