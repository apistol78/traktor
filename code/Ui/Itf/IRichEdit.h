/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IRichEdit_H
#define traktor_ui_IRichEdit_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{

class Color4ub;

	namespace ui
	{

/*! \brief RichEdit interface.
 * \ingroup UI
 */
class IRichEdit : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual int addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline) = 0;

	virtual void setAttribute(int start, int length, int attribute) = 0;

	virtual void clear(bool attributes, bool content) = 0;

	virtual void insert(const std::wstring& text) = 0;

	virtual int getCaretOffset() const = 0;

	virtual int getLineFromOffset(int offset) const = 0;

	virtual int getLineCount() const = 0;

	virtual int getLineOffset(int line) const = 0;

	virtual int getLineLength(int line) const = 0;

	virtual std::wstring getLine(int line) const = 0;
	
	virtual bool redo() = 0;
	
	virtual bool undo() = 0;
};

	}
}

#endif	// traktor_ui_IRichEdit_H
