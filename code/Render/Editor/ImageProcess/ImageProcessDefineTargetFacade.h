#ifndef traktor_render_ImageProcessDefineTargetFacade_H
#define traktor_render_ImageProcessDefineTargetFacade_H

#include "Render/Editor/ImageProcess/IImageProcessDefineFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessDefineTargetFacade : public IImageProcessDefineFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessDefine* definition) const;

	virtual std::wstring getText(const ImageProcessDefine* definition) const;
};

	}
}

#endif	// traktor_render_ImageProcessDefineTargetFacade_H
