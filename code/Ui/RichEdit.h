/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_RichEdit_H
#define traktor_ui_RichEdit_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Rich formatting editor.
 * \ingroup UI
 */
class T_DLLCLASS RichEdit : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"", int style = WsClientBorder);

	int addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline);

	void setAttribute(int start, int length, int attribute);

	void clear(bool attributes, bool content);

	void insert(const std::wstring& text);

	int getCaretOffset() const;

	int getLineFromOffset(int offset) const;

	int getLineCount() const;

	int getLineOffset(int line) const;

	int getLineLength(int line) const;

	std::wstring getLine(int line) const;
	
	bool redo();
	
	bool undo();
};

	}
}

#endif	// traktor_ui_RichEdit_H
