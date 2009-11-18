#ifndef traktor_ui_ClipboardWin32_H
#define traktor_ui_ClipboardWin32_H

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

class ClipboardWin32 : public IClipboard
{
public:
	ClipboardWin32();

	virtual void destroy();

	virtual bool setObject(ISerializable* object);

	virtual bool setText(const std::wstring& text);

	virtual ClipboardContentType getContentType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual std::wstring getText() const;

private:
	uint32_t m_objectFormat;
};

	}
}

#endif	// traktor_ui_ClipboardWin32_H
