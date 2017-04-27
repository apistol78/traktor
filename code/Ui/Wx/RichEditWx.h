/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_RichEditWx_H
#define traktor_ui_RichEditWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IRichEdit.h"

namespace traktor
{
	namespace ui
	{

class RichEditWx : public WidgetWxImpl< IRichEdit, wxTextCtrl >
{
public:
	RichEditWx(EventSubject* owner);

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
	std::vector< wxTextAttr > m_attributes;

	void onUpdate(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_RichEditWx_H
