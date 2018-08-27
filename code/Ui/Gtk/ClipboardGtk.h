/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ClipboardGtk_H
#define traktor_ui_ClipboardGtk_H

#include <string>
#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UI
 */
class ClipboardGtk : public IClipboard
{
public:
	virtual void destroy();

	virtual bool setObject(ISerializable* object);

	virtual bool setText(const std::wstring& text);

	virtual bool setImage(const drawing::Image* image);

	virtual ClipboardContentType getContentType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual std::wstring getText() const;

	virtual Ref< const drawing::Image > getImage() const;
};

	}
}

#endif	// traktor_ui_ClipboardGtk_H
