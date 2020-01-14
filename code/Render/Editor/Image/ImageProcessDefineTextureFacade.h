#pragma once

#include "Render/Editor/Image/IImageProcessDefineFacade.h"

namespace traktor
{
	namespace render
	{

class ImageProcessDefineTextureFacade : public IImageProcessDefineFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessDefine* definition) const override final;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessDefine* definition) const override final;
};

	}
}

