/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ClipboardX11_H
#define traktor_ui_ClipboardX11_H

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UI
 */
class ClipboardX11 : public IClipboard
{
public:
	ClipboardX11();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool setObject(ISerializable* object) T_OVERRIDE T_FINAL;

	virtual bool setText(const std::wstring& text) T_OVERRIDE T_FINAL;

	virtual bool setImage(const drawing::Image* image) T_OVERRIDE T_FINAL;

	virtual ClipboardContentType getContentType() const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > getObject() const T_OVERRIDE T_FINAL;

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;

	virtual Ref< const drawing::Image > getImage() const T_OVERRIDE T_FINAL;

private:
	ClipboardContentType m_type;
	Ref< ISerializable > m_object;
	std::wstring m_text;
	Ref< const drawing::Image > m_image;
};

	}
}

#endif	// traktor_ui_ClipboardX11_H
