#ifndef traktor_render_IImageProcessDefineFacade_H
#define traktor_render_IImageProcessDefineFacade_H

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class ImageProcessDefine;

class IImageProcessDefineFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessDefine* definition) const = 0;

	virtual std::wstring getText(const ImageProcessDefine* definition) const = 0;
};

	}
}

#endif	// traktor_render_IImageProcessDefineFacade_H
