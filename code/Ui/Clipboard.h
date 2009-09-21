#ifndef traktor_ui_Clipboard_H
#define traktor_ui_Clipboard_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

	namespace ui
	{

class IClipboard;

/*! \brief Clipboard
 * \ingroup UI
 */
class T_DLLCLASS Clipboard : public Object
{
	T_RTTI_CLASS(Clipboard)

public:
	Clipboard(IClipboard* clipboard);

	virtual ~Clipboard();

	void destroy();

	bool setObject(Serializable* object);

	bool setText(const std::wstring& text);

	ClipboardContentType getContentType() const;

	Serializable* getObject() const;

	std::wstring getText() const;

private:
	IClipboard* m_clipboard;
};

	}
}

#endif	// traktor_ui_Clipboard_H
