/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IImageProcessDefineFacade_H
#define traktor_render_IImageProcessDefineFacade_H

#include "Core/Object.h"

namespace traktor
{
	namespace editor
	{
	
class IEditor;

	}

	namespace render
	{

class ImageProcessDefine;

class IImageProcessDefineFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessDefine* definition) const = 0;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessDefine* definition) const = 0;
};

	}
}

#endif	// traktor_render_IImageProcessDefineFacade_H
