#ifndef traktor_render_ImageProcessDefineTextureFacade_H
#define traktor_render_ImageProcessDefineTextureFacade_H

#include "Render/Editor/ImageProcess/IImageProcessDefineFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessDefineTextureFacade : public IImageProcessDefineFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const ImageProcessDefine* definition) const T_OVERRIDE T_FINAL;

	virtual std::wstring getText(const ImageProcessDefine* definition) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ImageProcessDefineTextureFacade_H
