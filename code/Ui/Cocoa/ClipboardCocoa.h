/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ClipboardCocoa_H
#define traktor_ui_ClipboardCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

class ClipboardCocoa : public IClipboard
{
public:
	virtual ~ClipboardCocoa() {}

	virtual void destroy() T_OVERRIDE T_FINAL;
	
	virtual bool setObject(ISerializable* object) T_OVERRIDE T_FINAL;

	virtual bool setText(const std::wstring& text) T_OVERRIDE T_FINAL;

	virtual bool setImage(const drawing::Image* image) T_OVERRIDE T_FINAL;

	virtual ClipboardContentType getContentType() const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > getObject() const T_OVERRIDE T_FINAL;

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;

	virtual Ref< const drawing::Image > getImage() const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_ClipboardCocoa_H
