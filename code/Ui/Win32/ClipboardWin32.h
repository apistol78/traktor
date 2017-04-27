/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ClipboardWin32_H
#define traktor_ui_ClipboardWin32_H

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ClipboardWin32 : public IClipboard
{
public:
	ClipboardWin32();

	virtual void destroy();

	virtual bool setObject(ISerializable* object);

	virtual bool setText(const std::wstring& text);

	virtual bool setImage(const drawing::Image* image);

	virtual ClipboardContentType getContentType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual std::wstring getText() const;

	virtual Ref< const drawing::Image > getImage() const;

private:
	uint32_t m_objectFormat;
};

	}
}

#endif	// traktor_ui_ClipboardWin32_H
