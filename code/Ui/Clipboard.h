#ifndef traktor_ui_Clipboard_H
#define traktor_ui_Clipboard_H

#include "Core/Object.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace ui
	{

class IClipboard;

/*! \brief Clipboard
 * \ingroup UI
 */
class T_DLLCLASS Clipboard : public Object
{
	T_RTTI_CLASS;

public:
	Clipboard(IClipboard* clipboard);

	virtual ~Clipboard();

	void destroy();

	bool setObject(ISerializable* object);

	bool setText(const std::wstring& text);

	ClipboardContentType getContentType() const;

	Ref< ISerializable > getObject() const;

	std::wstring getText() const;

private:
	IClipboard* m_clipboard;
};

	}
}

#endif	// traktor_ui_Clipboard_H
