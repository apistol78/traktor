#ifndef traktor_world_IPostProcessDefineFacade_H
#define traktor_world_IPostProcessDefineFacade_H

#include "Core/Object.h"

namespace traktor
{
	namespace world
	{

class PostProcessDefine;

class IPostProcessDefineFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const PostProcessDefine* definition) const = 0;

	virtual std::wstring getText(const PostProcessDefine* definition) const = 0;
};

	}
}

#endif	// traktor_world_IPostProcessDefineFacade_H
