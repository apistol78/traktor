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
	virtual void destroy();
	
	virtual bool setObject(ISerializable* object);

	virtual bool setText(const std::wstring& text);

	virtual ClipboardContentType getContentType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual std::wstring getText() const;
};

	}
}

#endif	// traktor_ui_ClipboardCocoa_H
