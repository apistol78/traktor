#ifndef traktor_ui_RichEditWin32_H
#define traktor_ui_RichEditWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IRichEdit.h"

#if !defined(WINCE)
#	include <richedit.h>
#endif

namespace traktor
{
	namespace ui
	{

class RichEditWin32 : public WidgetWin32Impl< IRichEdit >
{
public:
	RichEditWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline);

	virtual void setAttribute(int start, int length, int attribute);

	virtual void clear(bool attributes, bool content);

	virtual void insert(const std::wstring& text);

	virtual int getCaretOffset() const;

	virtual int getLineFromOffset(int offset) const;

	virtual int getLineCount() const;

	virtual int getLineOffset(int line) const;

	virtual int getLineLength(int line) const;

	virtual std::wstring getLine(int line) const;
	
	virtual bool redo();
	
	virtual bool undo();

private:
#if !defined(WINCE)
	std::vector< CHARFORMAT2 > m_attributes;
#endif

	LRESULT onGetDlgCode(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_RichEditWin32_H
