/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessDefine* definition) const override final;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessDefine* definition) const override final;
};

	}
}

#endif	// traktor_render_ImageProcessDefineTextureFacade_H
