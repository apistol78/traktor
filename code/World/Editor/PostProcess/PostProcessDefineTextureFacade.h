#ifndef traktor_world_PostProcessDefineTextureFacade_H
#define traktor_world_PostProcessDefineTextureFacade_H

#include "World/Editor/PostProcess/IPostProcessDefineFacade.h"

namespace traktor
{
	namespace world
	{

class PostProcessDefineTextureFacade : public IPostProcessDefineFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const PostProcessDefine* definition) const;

	virtual std::wstring getText(const PostProcessDefine* definition) const;
};

	}
}

#endif	// traktor_world_PostProcessDefineTextureFacade_H
